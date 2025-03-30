#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <sstream>
#include <stdexcept>

// Random HEX colour
std::string rand_color() {
    const int min_brightness = 128;
    int r = min_brightness + rand() % (256 - min_brightness);
    int g = min_brightness + rand() % (256 - min_brightness);
    int b = min_brightness + rand() % (256 - min_brightness);

    char buffer[7];
    sprintf(buffer, "%02x%02x%02x", r, g, b);
    return std::string(buffer);
}

void print_E1_E2(std::ostream& os, int n) {
    for (int i = 1; i < n; i++) {
        for (int j = i; j <= n + 1; j++) {
            for (int k = i + 1; k <= n; k++) {
                os << "A_" << i << "_" << k << " -> B_" << i << "_" << j << "_" << k << ";\n";
                os << "B_" << i << "_" << j << "_" << k << " -> C_" << i << "_" << j << "_" << k << ";\n";
            }
        }
    }
}

void print_E3(std::ostream& os, int n) {
    for (int i_y = 1; i_y < n; i_y++) {
        int i_x = i_y + 1;
        int k_y = i_x;
        for (int j = i_x + 1; j <= n + 1; j++) {
            for (int k_x = i_x + 1; k_x <= n; k_x++) {
                os << "C_" << i_y << "_" << j << "_" << k_y << " -> B_" << i_x << "_" << j << "_" << k_x << ";\n";
            }
        }
    }
}

void print_E4(std::ostream& os, int n) {
    for (int i_y = 1; i_y < n - 1; i_y++) {
        int i_x = i_y + 1;
        int j = i_x;
        int k_y = i_x;

        for (int k_x = i_x + 1; k_x <= n; k_x++) {
            os << "C_" << i_y << "_" << j << "_" << k_y << " -> A_" << i_x << "_" << k_x << ";\n";
        }

        for (int k_y = i_x + 1; k_y <= n; k_y++) {
            int k_x = k_y;
            os << "C_" << i_y << "_" << j << "_" << k_y << " -> A_" << i_x << "_" << k_x << ";\n";
        }
    }
}

void print_E5(std::ostream& os, int n) {
    for (int i_1 = 1; i_1 < n - 1; i_1++) {
        int i_2 = i_1 + 1;
        for (int k = i_2 + 1; k <= n; k++) {
            for (int j = i_2 + 1; j <= n + 1; j++) {
                os << "C_" << i_1 << "_" << j << "_" << k << " -> C_" << i_2 << "_" << j << "_" << k << ";\n";
            }
        }
    }
}

void color_graph(std::ostream& os, int n) {
    for (int i = 1; i < n; i++) {
        std::string color_A = rand_color();
        std::string color_B = rand_color();
        std::string color_C = rand_color();

        for (int k = i + 1; k <= n; k++) {
            os << "A_" << i << "_" << k
               << " [label=<A<sub>" << i << "," << k << "</sub>>, fillcolor=\"#" << color_A << "\", style=filled];\n";
            for (int j = i; j <= n + 1; j++) {
                os << "C_" << i << "_" << j << "_" << k
                   << " [label=<C<sub>" << i << "," << j << "," << k << "</sub>>, fillcolor=\"#" << color_C << "\", style=filled];\n";
                os << "B_" << i << "_" << j << "_" << k
                   << " [label=<B<sub>" << i << "," << j << "," << k << "</sub>>, fillcolor=\"#" << color_B << "\", style=filled];\n";
            }
        }
    }
}

void generate_graph_dot(const std::string& filename, int n) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }

    ofs << "digraph {\nsize=\"20,12\"\n";

    print_E1_E2(ofs, n);
    print_E3(ofs, n);
    print_E4(ofs, n);
    print_E5(ofs, n);
    color_graph(ofs, n);

    ofs << "}\n";
}

int main(int argc, char* argv[]) {
    try {
        srand(static_cast<unsigned>(time(nullptr)));
        int n = 3; // standard value

        if (argc > 1) {
            n = std::stoi(argv[1]);
            if (n <= 1) {
                throw std::invalid_argument("Please provide a number greater than 1.");
            }
        }

        std::string dot_file = "diekert.dot";
        std::string png_file = "diekert.png";

        generate_graph_dot(dot_file, n);

        // create PNG with Graphviz
        std::ostringstream cmd;
        cmd << "dot " << dot_file << " -Tpng -o " << png_file;
        if (std::system(cmd.str().c_str()) != 0) {
            throw std::runtime_error("Error generating PNG using Graphviz.");
        }

        std::cout << "Graph successfully generated: " << png_file << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
