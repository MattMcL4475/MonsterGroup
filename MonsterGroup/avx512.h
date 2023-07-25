#ifndef AVX512_H
#define AVX512_H

void complexMultiplyAVX512(double* a_re, double* a_im, double* b_re, double* b_im, double* c_re, double* c_im, size_t num_complex);
#endif
