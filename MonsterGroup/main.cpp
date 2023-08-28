#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <thread>
#include "utility.h"
#include "matrix.h"

int main(int argc, char* argv[]) {
    size_t N = 1024; //196883

    if (argc == 2) {
        N = std::atoi(argv[1]);
    }
    else if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [N]\n";
        return 1;
    }

    const double oneGiB = 1024.0 * 1024.0 * 1024.0;

    // each matrix element has a real and imaginary double.  2 matrices x NxN.
    double memNeeded = 2 * sizeof(double) * 2 * N * N  / oneGiB;

    double totalSystemMemGiB = getSystemMemory();

    std::cout << "Memory required: " << memNeeded << " GiB; Available: " << totalSystemMemGiB << " GiB.\n";

    if (memNeeded > totalSystemMemGiB) {
        std::cout << "Insufficient memory. Required: " << memNeeded
            << " GiB, Available: " << totalSystemMemGiB << " GiB.\n";
        return 1;
    }

    unsigned int nthreads = std::thread::hardware_concurrency();
    std::cout << "std::thread::hardware_concurrency() = " << nthreads << "\n";
    double cpuClockSpeedGHz = getCPUClockSpeed();

    // Estimate the number of floating-point operations for matrix multiplication
    double flopsPerMult = 6.0 * N * N * N; // 4 mult 2 add for complex numbers

    // Estimate the number of FLOPs that can be done per second
    double flopsPerSecond = cpuClockSpeedGHz * 1e9 * nthreads;

    // Estimate the time required in seconds
    double estimatedTimeSeconds = flopsPerMult / flopsPerSecond;

    std::cout << "Theoretical time to completion: " << estimatedTimeSeconds << " seconds.\n";

    auto start = std::chrono::system_clock::now();

    printTimeAndRestart(start, "Start");
    Matrix matrixA(N, N);
    //Matrix matrixB(N, N);

    std::stringstream ss;
    ss.imbue(std::locale(""));
    //ss << "Memory allocation for 2 x [" << N << " x " << N << " SoA matrix with " << N * N << " total elements]";
    ss << "Memory allocation for 1 x [" << N << " x " << N << " SoA matrix with " << N * N << " total elements]";
    std::string message = ss.str();
    printTimeAndRestart(start, message);

    matrixA.fill();
    //matrixB.fill();
    //printTimeAndRestart(start, "Filled both matrices with random numbers");
    printTimeAndRestart(start, "Filled matrix with random numbers");

    //matrixA.multiplyWith(matrixB);
    //printTimeAndRestart(start, "Multplication");
}

