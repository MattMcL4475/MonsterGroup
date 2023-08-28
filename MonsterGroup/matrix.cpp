#include <immintrin.h>
#include <cassert>
#include <vector>
#include <random>
#include <thread>
#include <iostream>
#include <complex>
#include <new>
#include <cstdlib> 
#include "matrix.h"
#include "utility.h"
#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h> // _aligned_malloc https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-malloc?view=msvc-170
#endif

Matrix::Matrix(int rows, int cols) : rows(rows), cols(cols)
{
#if defined(_WIN32) || defined(_WIN64)
    real = static_cast<double*>(_aligned_malloc(rows * cols * sizeof(double), 64));
#else
    real = static_cast<double*>(aligned_alloc(64, rows * cols * sizeof(double)));
#endif
    if (!real) throw std::bad_alloc();

#if defined(_WIN32) || defined(_WIN64)
    imag = static_cast<double*>(_aligned_malloc(rows * cols * sizeof(double), 64));
#else
    imag = static_cast<double*>(aligned_alloc(64, rows * cols * sizeof(double)));
#endif
    if (!imag)
    {
#if defined(_WIN32) || defined(_WIN64)
        _aligned_free(real);
#else
        std::free(real);
#endif
        throw std::bad_alloc();
    }
}

Matrix::~Matrix()
{
#if defined(_WIN32) || defined(_WIN64)
    _aligned_free(real);
    _aligned_free(imag);
#else
    std::free(real);
    std::free(imag);
#endif
}

void Matrix::multiplyWith(const Matrix& other) {
    assert(cols == other.rows);

    double* newReal;
    double* newImag;
#if defined(_WIN32) || defined(_WIN64)
    newReal = static_cast<double*>(_aligned_malloc(rows * other.cols * sizeof(double), 64));
    newImag = static_cast<double*>(_aligned_malloc(rows * other.cols * sizeof(double), 64));
#else
    newReal = static_cast<double*>(aligned_alloc(64, rows * other.cols * sizeof(double)));
    newImag = static_cast<double*>(aligned_alloc(64, rows * other.cols * sizeof(double)));
#endif
    if (!newReal || !newImag) throw std::bad_alloc();

    std::fill_n(newReal, rows * other.cols, 0.0);
    std::fill_n(newImag, rows * other.cols, 0.0);

#if defined(__AVX512F__)
    const int vectorSize = 8; // AVX-512 works on 8 doubles at a time
    unsigned int nthreads = std::thread::hardware_concurrency() / 2; // Assumes only one AVX-512 core per hyperthreaded chip
    std::vector<std::thread> threads;
    threads.reserve(nthreads);

    for (unsigned int n = 0; n < nthreads; ++n) {
        threads.emplace_back([this, &other, newReal, newImag, n, nthreads, vectorSize] {
            int start = (rows * n) / nthreads;
            int end = (rows * (n + 1)) / nthreads;

            for (int i = start; i < end; ++i) {
                for (int j = 0; j < other.cols; j += vectorSize) {
                    __m512d sumReal = _mm512_setzero_pd();
                    __m512d sumImag = _mm512_setzero_pd();
                    int k = 0;

                    int unrollLimit = (cols / 4) * 4; // Compute the limit where loop unrolling can be safely applied

                    // Unrolled loop for groups of 4
                    for (; k < unrollLimit; k += 4) {
                        __m512d aReal[4], aImag[4], bReal[4], bImag[4];

                        // Prefetching hints (optional)
                        _mm_prefetch((char*)&other.real[(k + 4) * other.cols + j], _MM_HINT_T0);
                        _mm_prefetch((char*)&other.imag[(k + 4) * other.cols + j], _MM_HINT_T0);

                        for (int u = 0; u < 4; ++u) {
                            aReal[u] = _mm512_set1_pd(real[i * cols + k + u]);
                            aImag[u] = _mm512_set1_pd(imag[i * cols + k + u]);
                            bReal[u] = _mm512_load_pd(&other.real[(k + u) * other.cols + j]);
                            bImag[u] = _mm512_load_pd(&other.imag[(k + u) * other.cols + j]);
                            sumReal = _mm512_fmadd_pd(aReal[u], bReal[u], _mm512_fnmadd_pd(aImag[u], bImag[u], sumReal));
                            sumImag = _mm512_fmadd_pd(aReal[u], bImag[u], _mm512_fmadd_pd(aImag[u], bReal[u], sumImag));
                        }
                    }

                    // Handle remaining iterations (if cols is not divisible by 4)
                    for (; k < cols; ++k) {
                        __m512d aReal = _mm512_set1_pd(real[i * cols + k]);
                        __m512d aImag = _mm512_set1_pd(imag[i * cols + k]);
                        __m512d bReal = _mm512_load_pd(&other.real[k * other.cols + j]);
                        __m512d bImag = _mm512_load_pd(&other.imag[k * other.cols + j]);

                        sumReal = _mm512_fmadd_pd(aReal, bReal, _mm512_fnmadd_pd(aImag, bImag, sumReal));
                        sumImag = _mm512_fmadd_pd(aReal, bImag, _mm512_fmadd_pd(aImag, bReal, sumImag));
                    }

                    _mm512_store_pd(&newReal[i * other.cols + j], sumReal); // Aligned store
                    _mm512_store_pd(&newImag[i * other.cols + j], sumImag); // Aligned store
                }

                // Handle the remaining columns
                for (int j = (other.cols / vectorSize) * vectorSize; j < other.cols; ++j) {
                    double sumReal = 0.0;
                    double sumImag = 0.0;

                    for (int k = 0; k < cols; ++k) {
                        double aReal = real[i * cols + k];
                        double aImag = imag[i * cols + k];
                        double bReal = other.real[k * other.cols + j];
                        double bImag = other.imag[k * other.cols + j];
                        sumReal += aReal * bReal - aImag * bImag;
                        sumImag += aReal * bImag + aImag * bReal;
                    }

                    newReal[i * other.cols + j] = sumReal;
                    newImag[i * other.cols + j] = sumImag;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
#else
    // Non-AVX-512 code
    unsigned int nthreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    threads.reserve(nthreads);

    int cacheLineSize = getCacheLineSize();
    int tileSize = cacheLineSize > 0 ? cacheLineSize * 4 : 32;

    for (unsigned int n = 0; n < nthreads; ++n) {
        threads.emplace_back([this, &other, newReal, newImag, n, nthreads, tileSize] {
            int start = (rows * n) / nthreads;
            int end = (rows * (n + 1)) / nthreads;

            for (int ii = start; ii < end; ii += tileSize) {
                for (int kk = 0; kk < cols; kk += tileSize) {
                    for (int jj = 0; jj < other.cols; jj += tileSize) {
                        for (int i = ii; i < std::min(ii + tileSize, rows); ++i) {
                            for (int k = kk; k < std::min(kk + tileSize, cols); ++k) {
                                double aReal = real[i * cols + k];
                                double aImag = imag[i * cols + k];
                                int kOtherCols = k * other.cols;
                                int iOtherCols = i * other.cols;
                                for (int j = jj; j < std::min(jj + tileSize, other.cols); ++j) {
                                    int resultIndex = iOtherCols + j;
                                    double bReal = other.real[kOtherCols + j];
                                    double bImag = other.imag[kOtherCols + j];

                                    newReal[resultIndex] += aReal * bReal - aImag * bImag;
                                    newImag[resultIndex] += aReal * bImag + aImag * bReal;
                                }
                            }
                        }
                    }
                }
            }
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }
#endif

    // Free existing arrays
#if defined(_WIN32) || defined(_WIN64)
    _aligned_free(real);
    _aligned_free(imag);
#else
    std::free(real);
    std::free(imag);
#endif

    // Replace the existing arrays with the new ones
    real = newReal;
    imag = newImag;
    cols = other.cols; // Update the column size
}

std::complex<double> Matrix::get(int i, int j) const {
    return { real[i * cols + j], imag[i * cols + j] };
}

void Matrix::set(int i, int j, const std::complex<double>& val) {
    real[i * cols + j] = val.real();
    imag[i * cols + j] = val.imag();
}

void Matrix::fill() {
    unsigned int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(numThreads);
    size_t rowsPerThread = rows / numThreads;

    for (size_t i = 0; i < numThreads; ++i)
    {
        size_t startRow = i * rowsPerThread;
        size_t endRow = (i == (numThreads - 1)) ? rows : (startRow + rowsPerThread);
        threads[i] = std::thread([this, startRow, endRow] {
            std::random_device rd; // Random seed
            std::mt19937 gen(rd()); // Mersenne Twister engine
            std::uniform_real_distribution<> dist(-1.0, 1.0); // Uniform distribution between -1.0 and 1.0

            for (size_t row = startRow; row < endRow; ++row) {
                for (int col = 0; col < cols; ++col) {
                    real[row * cols + col] = dist(gen);
                    imag[row * cols + col] = dist(gen);
                }
            }
            });
    }

    for (auto& th : threads) th.join();
}