#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <complex>
#include <cstddef>

struct Matrix {
    double* real;
    double* imag;
    int rows, cols;

    Matrix(int rows, int cols);
    ~Matrix();

    std::complex<double> get(int i, int j) const;
    void set(int i, int j, const std::complex<double>& val);
    void fill();
    void multiplyWith(const Matrix& other);
};

#endif // MATRIX_H
