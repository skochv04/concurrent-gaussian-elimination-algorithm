# 🔄 Сoncurrent Gaussian Elimination Algorithm
An implementation of the concurrent Gaussian elimination algorithm, developed as part of the Concurrency Theory course at AGH University of Science and Technology. This project also applies the theory of traces for thread scheduling in the concurrent Gaussian elimination process.

## 🔢 The task involves the following steps (for a matrix of size N):
- Identify indivisible tasks in the algorithm, name them, and construct an alphabet in terms of trace theory.
- Construct a dependency relation for the alphabet describing the Gaussian elimination algorithm.
- Represent the Gaussian elimination algorithm as a sequence of alphabet symbols.
- Generate the Diekert dependency graph.
- Convert the sequence of symbols describing the algorithm to its Foata normal form.

For a detailed report on the implementation and the steps taken, please refer to the [report.pdf](task/report.pdf).

## 🚀 How to Run the Application?

### 1️⃣ Install dependencies
Before running the program, you need to install Graphviz:
```bash
sudo apt install graphviz
```

### 2️⃣ Run the Diekert graph generator (optional)
To run the Diekert graph generator located in the app folder, use the following commands:
```bash
g++ -o diekert app/diekert.cpp
./diekert n
```
where n is the size of the matrix.

### 3️⃣ Run the concurrent Gaussian elimination calculations
To run the concurrent Gaussian elimination calculations, also located in the app folder, use the following commands:
```bash
g++ app/elimination.cpp -o elimination
./elimination file
```
where file is an optional input file, e.g., "in.txt". By default, the file "in.txt" will be used.
