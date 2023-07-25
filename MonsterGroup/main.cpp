#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <thread>
#include "utility.h"
#include "matrix.h"

//const size_t N = 196883;
const size_t N = 10000;

int main()
{
    double memNeeded = N * N * sizeof(std::complex<double>) / (1024.0 * 1024.0 * 1024.0);
    double totalSystemMemGiB = getSystemMemory();

    std::cout << "Memory required: " << memNeeded << " GiB; Available: " << totalSystemMemGiB << " GiB.\n";

    if (memNeeded > totalSystemMemGiB) {
        std::cout << "Insufficient memory. Required: " << memNeeded
            << " GiB, Available: " << totalSystemMemGiB << " GiB.\n";
        return 1;
    }

    auto start = std::chrono::system_clock::now();
    printExecutionTime(start, "Start");
    std::vector<std::vector<std::complex<double>>> matrixA(N, std::vector<std::complex<double>>(N));
    printExecutionTime(start, "Memory allocation for " + std::to_string(N) + "x" + std::to_string(N) + " matrix");
    fillMatrixWhole(matrixA, N);
    printExecutionTime(start, "MatrixA filled");
}

