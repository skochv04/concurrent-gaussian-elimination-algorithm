#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <stdexcept>
#include <fstream>

class ThreadData {
public:
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cond;
    bool start_flag = false;
    bool finished_flag = false;
    bool exit_flag = false;

    ThreadData() = default;
    ~ThreadData() {
        if (thread.joinable()) {
            thread.join();
        }
    }
};

class MatrixSolver {
private:
    size_t matrix_size;
    std::vector<std::vector<double>> M;
    std::vector<std::vector<double>> m;
    std::vector<std::vector<std::vector<double>>> n;
    std::vector<std::unique_ptr<ThreadData>> thread_pool;
    size_t actual_row_index = 0;
    char current_action = 'A';

public:
    explicit MatrixSolver(size_t size)
            : matrix_size(size),
              M(size, std::vector<double>(size + 1)),
              m(size, std::vector<double>(size)),
              n(size, std::vector<std::vector<double>>(size + 1, std::vector<double>(size))) {}

    void allocate_threads(size_t thread_count) {
        thread_pool.reserve(thread_count);
        for (size_t i = 0; i < thread_count; ++i) {
            auto data = std::make_unique<ThreadData>();
            data->thread = std::thread(&MatrixSolver::thread_main, this, i);
            thread_pool.push_back(std::move(data));
        }
    }

    void perform_calculation(char action, size_t k, size_t j, size_t i) {
        --k; --j; --i;
        if (action == 'A') {
            m[k][i] = M[k][i] / M[i][i];
        } else if (action == 'B') {
            n[k][j][i] = M[i][j] * m[k][i];
        } else if (action == 'C') {
            M[k][j] -= n[k][j][i];
        }
    }

    void perform_action(size_t thread_id) {
        size_t i = actual_row_index;
        size_t k, j;

        if (current_action == 'A') {
            k = i + 1 + thread_id;
            perform_calculation('A', k, 0, i);
        } else {
            k = i + 1 + thread_id / (matrix_size + 2 - i);
            j = i + thread_id % (matrix_size + 2 - i);
            perform_calculation(current_action, k, j, i);
        }
    }

    void signal_thread_start(size_t thread_index) {
        std::unique_lock<std::mutex> lock(thread_pool[thread_index]->mutex);
        thread_pool[thread_index]->start_flag = true;
        thread_pool[thread_index]->cond.notify_one();
    }

    void wait_thread_finish(size_t thread_index) {
        std::unique_lock<std::mutex> lock(thread_pool[thread_index]->mutex);
        thread_pool[thread_index]->cond.wait(lock, [&]() { return thread_pool[thread_index]->finished_flag; });
        thread_pool[thread_index]->finished_flag = false;
    }

    void run_threads(size_t threads_amount) {
        for (size_t i = 0; i < threads_amount; ++i) {
            signal_thread_start(i);
        }
        for (size_t i = 0; i < threads_amount; ++i) {
            wait_thread_finish(i);
        }
    }

    void thread_main(size_t thread_id) {
        while (true) {
            {
                std::unique_lock<std::mutex> lock(thread_pool[thread_id]->mutex);
                thread_pool[thread_id]->cond.wait(lock, [&]() { return thread_pool[thread_id]->start_flag || thread_pool[thread_id]->exit_flag; });
                if (thread_pool[thread_id]->exit_flag) break;
                thread_pool[thread_id]->start_flag = false;
            }
            perform_action(thread_id);
            {
                std::lock_guard<std::mutex> lock(thread_pool[thread_id]->mutex);
                thread_pool[thread_id]->finished_flag = true;
                thread_pool[thread_id]->cond.notify_one();
            }
        }
    }

    void signal_threads_exit() {
        for (auto& thread_data : thread_pool) {
            {
                std::lock_guard<std::mutex> lock(thread_data->mutex);
                thread_data->exit_flag = true;
                thread_data->cond.notify_one();
            }
        }
    }

    void schedule() {
        for (actual_row_index = 1; actual_row_index < matrix_size; ++actual_row_index) {
            size_t remaining_rows = matrix_size - actual_row_index;
            size_t remaining_cells = remaining_rows * (matrix_size + 2 - actual_row_index);

            for (char action = 'A'; action <= 'C'; ++action) {
                current_action = action;
                run_threads(action == 'A' ? remaining_rows : remaining_cells);
            }
        }
    }

    void print_M(std::ostream& output_file) const {
        output_file << matrix_size << "\n";
        for (const auto& row : M) {
            for (size_t j = 0; j < row.size() - 1; ++j) {
                output_file << row[j] << " ";
            }
            output_file << "\n";
        }

        for (size_t i = 0; i < M.size(); ++i) {
            output_file << M[i].back() << (i < M.size() - 1 ? " " : "\n");
        }
    }

    void solve(std::istream& input, std::ostream& output) {
        for (size_t i = 0; i < matrix_size; ++i) {
            for (size_t j = 0; j < matrix_size; ++j) {
                if (!(input >> M[i][j])) throw std::runtime_error("Error reading matrix");
            }
        }

        for (size_t i = 0; i < matrix_size; ++i) {
            if (!(input >> M[i][matrix_size])) throw std::runtime_error("Error reading vector");
        }

        schedule();

        signal_threads_exit();

        for (size_t i = matrix_size - 1; i < matrix_size; --i) {
            M[i][matrix_size] /= M[i][i];
            M[i][i] = 1.0;

            for (size_t k = 0; k < i; ++k) {
                M[k][matrix_size] -= M[k][i] * M[i][matrix_size];
                M[k][i] = 0.0;
            }
        }

        for (size_t j = 0; j < matrix_size; ++j) {
            for (size_t i = j + 1; i < matrix_size; ++i) {
                M[i][j] = 0.0;
            }
        }
        print_M(output);
    }
};

int main(int argc, char** argv) {
    try {
        std::string input_filename = "in.txt";
        std::string output_filename = "output.txt";

        if (argc > 1 && argv[1][0] != '\0') {
            input_filename = argv[1];
        }

        std::ofstream output_file("test_output.txt");

        std::ifstream input_file(input_filename);
        if (!input_file) throw std::runtime_error("Error: Could not open input file: " + input_filename);
        if (!output_file) throw std::runtime_error("Error: Could not open output file.");

        size_t matrix_size;
        input_file >> matrix_size;

        if (matrix_size == 0 || matrix_size > (1 << (sizeof(size_t) * 2))) {
            throw std::runtime_error("Error: Invalid matrix size.");
        }

        std::cout << "Successfully read file: " << input_filename << std::endl;

        MatrixSolver solver(matrix_size);
        solver.allocate_threads((matrix_size - 1) * (matrix_size + 1));
        solver.solve(input_file, output_file);

        std::cout << "Successfully wrote solution to output file: " << output_filename << std::endl;

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}