#include <immintrin.h>
#include <cassert>
#include <vector>
#include <complex>
#include "avx512.h"
#include <random>
#include <thread>

// In-place schoolbook matrix multiplication w/ AVX512
void multiplyMatricesInPlaceWithAVX512(std::vector<std::vector<std::complex<double>>>& matrixA, std::vector<std::vector<std::complex<double>>>& matrixB)
{
    size_t N = matrixA.size();
    assert(N == matrixA[0].size() && N == matrixB.size() && N == matrixB[0].size());

    // Initialize temporary arrays for complex multiplication
    double a_re[8], a_im[8], b_re[8], b_im[8], c_re[8], c_im[8];

    for (size_t i = 0; i < N; ++i)
    {
        std::vector<std::complex<double>> row(N);

        for (size_t j = 0; j < N; ++j)
        {
            std::complex<double> sum(0.0, 0.0);
            size_t k = 0;

            // Process 8 complex numbers at a time
            for (; k + 8 <= N; k += 8)
            {
                // Load complex numbers into temporary arrays
                for (size_t l = 0; l < 8; ++l)
                {
                    a_re[l] = matrixA[i][k + l].real();
                    a_im[l] = matrixA[i][k + l].imag();
                    b_re[l] = matrixB[k + l][j].real();
                    b_im[l] = matrixB[k + l][j].imag();
                }

                // Multiply complex numbers
                complexMultiplyAVX512(a_re, a_im, b_re, b_im, c_re, c_im, 8);

                // Accumulate results
                for (size_t l = 0; l < 8; ++l)
                {
                    sum += std::complex<double>(c_re[l], c_im[l]);
                }
            }

            // Handle the remaining complex numbers if N is not a multiple of 8
            for (; k < N; ++k)
            {
                sum += matrixA[i][k] * matrixB[k][j];
            }

            row[j] = sum;
        }

        // Copy the result back into matrixA
        matrixA[i] = row;
    }
}



// Schoolbook matrix multiplication
void multiplyMatricesInPlace(std::vector<std::vector<std::complex<double>>>& matrixA, std::vector<std::vector<std::complex<double>>>& matrixB)
{
    size_t N = matrixA.size();
    assert(N == matrixA[0].size() && N == matrixB.size() && N == matrixB[0].size());

    std::vector<std::vector<std::complex<double>>> result(N, std::vector<std::complex<double>>(N, 0.0));

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            std::complex<double> sum(0.0, 0.0);
            for (size_t k = 0; k < N; ++k)
            {
                sum += matrixA[i][k] * matrixB[k][j];
            }
            result[i][j] = sum;
        }
    }

    matrixA = result;
}

void fillMatrixPart(std::vector<std::vector<std::complex<double>>>& matrix, size_t start, size_t end, unsigned seed, size_t N)
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

void fillMatrixWhole(std::vector<std::vector<std::complex<double>>>& matrix, size_t N)
{
    unsigned int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(numThreads);
    size_t rowsPerThread = N / numThreads;
    unsigned seed = time(nullptr);

    // Fill matrixA
    for (size_t i = 0; i < numThreads; ++i)
    {
        size_t startRow = i * rowsPerThread;
        size_t endRow = (i == (numThreads - 1)) ? N : (startRow + rowsPerThread);
        threads[i] = std::thread(fillMatrixPart, std::ref(matrix), startRow, endRow, seed + i, N);
    }

    for (auto& th : threads) th.join(); // Ensure all threads finish
}






