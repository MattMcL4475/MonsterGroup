#ifndef MATRIX_H
#define MATRIX_H

#include <immintrin.h>
#include <cassert>
#include <vector>
#include <complex>
#include "avx512.h"

void fillMatrixWhole(std::vector<std::vector<std::complex<double>>>& matrix, size_t N);

void multiplyMatricesInPlaceWithAVX512(std::vector<std::vector<std::complex<double>>>& matrixA, std::vector<std::vector<std::complex<double>>>& matrixB);

void multiplyMatricesInPlace(std::vector<std::vector<std::complex<double>>>& matrixA, std::vector<std::vector<std::complex<double>>>& matrixB);

#endif
