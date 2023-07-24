#include <cstdio>
#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <thread>
#include <chrono>
#include <ctime>
#include <fstream>
#include <string>
#include <iomanip>

const size_t N = 196883;

// Standard E96 - 24ads v5(24 vcpus, 672 GiB memory)
// Ubuntu Server 22.04 LTS - Gen2
// x64
// 0.6464 USD / hr
// 1.  sudo apt update && sudo apt install g++ -y
// 2.  nano monster.cpp (copy this file in)
// 3.  g++ -o monster monster.cpp
// 4.  ./monster

double getSystemMemory()
{
    std::ifstream file("/proc/meminfo");
    std::string line;

    while (getline(file, line))
    {
        if (line.find("MemTotal") != std::string::npos)
        {
            int pos = line.find(":");
            std::string memStr = line.substr(pos + 1);
            return std::stod(memStr) / (1024 * 1024);
        }
    }
    return 0.0;
}

double getProcessMemory()
{
    std::ifstream file("/proc/self/status");
    std::string line;

    while (getline(file, line))
    {
        if (line.find("VmRSS") != std::string::npos)
        {
            int pos = line.find(":");
            std::string memStr = line.substr(pos + 1);
            return std::stod(memStr) / (1024 * 1024);
        }
    }
    return 0.0;
}

void printExecutionTime(std::chrono::system_clock::time_point& start, const std::string& message)
{
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    int days = int(elapsed_seconds.count()) / (60 * 60 * 24);
    int hours = (int(elapsed_seconds.count()) / (60 * 60)) % 24;
    int minutes = (int(elapsed_seconds.count()) / 60) % 60;
    int seconds = int(elapsed_seconds.count()) % 60;

    // Extract milliseconds from the elapsed time
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds).count() % 1000;

    // Get system and process memory information
    double totalSystemMemGiB = getSystemMemory();
    double processMemGiB = getProcessMemory();

    std::cout << message << " execution time: ";
    if (days > 0) {
        std::cout << days << "d ";
    }
    if (hours > 0) {
        std::cout << hours << "h ";
    }
    if (minutes > 0) {
        std::cout << minutes << "m ";
    }
    if (seconds > 0 || milliseconds > 0) {
        std::cout << seconds << "s ";
    }
    if (milliseconds > 0) {
        std::cout << std::setw(3) << std::setfill('0') << milliseconds << "ms ";
    }
    std::cout << "\n";
    std::cout << "Memory usage: " << processMemGiB << " GiB / " << totalSystemMemGiB << " GiB (total)\n";

    start = std::chrono::system_clock::now();
}

void fillMatrix(std::vector<std::vector<std::complex<double>>>& matrix, size_t start, size_t end, unsigned seed)
{
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(-1.0, 1.0);

    for (size_t i = start; i < end; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            double realPart = distribution(generator);
            double imaginaryPart = distribution(generator);
            std::complex<double> complexNumber(realPart, imaginaryPart);
            matrix[i][j] = complexNumber;
        }
    }
}

// Multiply section of matrixA and matrixB, result is stored in matrixA
void multiplyMatrices(std::vector<std::vector<std::complex<double>>>& matrixA, std::vector<std::vector<std::complex<double>>>& matrixB,
    size_t startRow, size_t endRow)
{
    for (size_t row = startRow; row < endRow; ++row)
    {
        std::vector<std::complex<double>> resultRow(N); // Temporary row for storing the result

        for (size_t col = 0; col < N; ++col)
        {
            for (size_t k = 0; k < N; ++k)
            {
                resultRow[col] += matrixA[row][k] * matrixB[k][col];
            }
        }

        // Overwrite the original row in matrixA with the result
        matrixA[row] = resultRow;
    }
}

// initializeAndMultiplyMatrices(threads, numThreads, rowsPerThread, seed, start, matrixA);

void initializeAndMultiplyMatrices(std::vector<std::thread>& threads, unsigned int numThreads, const size_t& rowsPerThread, unsigned int seed, std::chrono::_V2::system_clock::time_point& start, std::vector<std::vector<std::complex<double>>>& matrixA)
{
    std::vector<std::vector<std::complex<double>>> matrixB(N, std::vector<std::complex<double>>(N));
    size_t multiplicationStepCount = 1000;

    for (size_t multiplicationStep = 0; multiplicationStep < multiplicationStepCount; ++multiplicationStep)
    {
        // 1.  Fill MatrixB
        threads = std::vector<std::thread>(numThreads);
        for (size_t i = 0; i < numThreads; ++i)
        {
            size_t startRow = i * rowsPerThread;
            size_t endRow = (i == (numThreads - 1)) ? N : (startRow + rowsPerThread);
            threads[i] = std::thread(fillMatrix, std::ref(matrixB), startRow, endRow, seed + numThreads + i);
        }
        for (auto& th : threads) th.join(); // Ensure all threads finish
        printExecutionTime(start, "MatrixB filled [" + std::to_string(multiplicationStep + 1) + " of " + std::to_string(multiplicationStepCount) + "]");

        // 2.  Multiply matrix A in place
        threads = std::vector<std::thread>(numThreads);
        for (size_t i = 0; i < numThreads; ++i)
        {
            size_t startRow = i * rowsPerThread;
            size_t endRow = (i == (numThreads - 1)) ? N : (startRow + rowsPerThread);
            threads[i] = std::thread(multiplyMatrices, std::ref(matrixA), std::ref(matrixB), startRow, endRow);
        }
        for (auto& th : threads) th.join(); // Ensure all threads finish
        printExecutionTime(start, "Matrices multiplied [" + std::to_string(multiplicationStep + 1) + " of " + std::to_string(multiplicationStepCount) + "]");
    }
}

int main()
{
    // Calculate the memory needed in GiB
    double memNeeded = N * N * sizeof(std::complex<double>) / (1024.0 * 1024.0 * 1024.0);
    double totalSystemMemGiB = getSystemMemory();

    std::cout << "Memory required: " << memNeeded << " GiB; Available: " << totalSystemMemGiB << " GiB.\n";

    if (memNeeded > totalSystemMemGiB) {
        std::cout << "Insufficient memory. Required: " << memNeeded
            << " GiB, Available: " << totalSystemMemGiB << " GiB.\n";
        return 1; // exit with non-zero status to indicate error
    }

    auto start = std::chrono::system_clock::now();
    printExecutionTime(start, "Start");

    // Create matrix and fill with random values
    std::vector<std::vector<std::complex<double>>> matrixA(N, std::vector<std::complex<double>>(N));
    printExecutionTime(start, "Memory allocation for " + std::to_string(N) + "x" + std::to_string(N) + " matrix");

    unsigned int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(numThreads);
    size_t rowsPerThread = N / numThreads;
    unsigned seed = time(nullptr);

    // Fill matrixA
    for (size_t i = 0; i < numThreads; ++i)
    {
        size_t startRow = i * rowsPerThread;
        size_t endRow = (i == (numThreads - 1)) ? N : (startRow + rowsPerThread);
        threads[i] = std::thread(fillMatrix, std::ref(matrixA), startRow, endRow, seed + i);
    }

    for (auto& th : threads) th.join(); // Ensure all threads finish
    printExecutionTime(start, "MatrixA filled");
}

