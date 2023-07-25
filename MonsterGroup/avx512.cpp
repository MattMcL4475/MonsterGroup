#include <cstddef>
#include <cassert>
#include "avx512.h"

void complexMultiplyAVX512(double* a_re, double* a_im, double* b_re, double* b_im, double* c_re, double* c_im, size_t num_complex)
{
    assert(a_re != nullptr && a_im != nullptr && b_re != nullptr && b_im != nullptr && c_re != nullptr && c_im != nullptr);

    size_t i = 0;

#ifdef __AVX512F__
    // Process 8 complex numbers at a time
    for (; i + 8 <= num_complex; i += 8) {
        __m512d va_re, va_im, vb_re, vb_im;

        if (((size_t)(a_re + i) % 64 == 0) && ((size_t)(a_im + i) % 64 == 0) && ((size_t)(b_re + i) % 64 == 0) && ((size_t)(b_im + i) % 64 == 0)) {
            // If the memory is aligned, use aligned load
            va_re = _mm512_load_pd(a_re + i);
            va_im = _mm512_load_pd(a_im + i);
            vb_re = _mm512_load_pd(b_re + i);
            vb_im = _mm512_load_pd(b_im + i);
        }
        else {
            // If the memory is not aligned, use unaligned load
            va_re = _mm512_loadu_pd(a_re + i);
            va_im = _mm512_loadu_pd(a_im + i);
            vb_re = _mm512_loadu_pd(b_re + i);
            vb_im = _mm512_loadu_pd(b_im + i);
        }

        // c = a * b
        __m512d vc_re = _mm512_sub_pd(_mm512_mul_pd(va_re, vb_re), _mm512_mul_pd(va_im, vb_im));
        __m512d vc_im = _mm512_add_pd(_mm512_mul_pd(va_re, vb_im), _mm512_mul_pd(va_im, vb_re));

        if ((size_t)(c_re + i) % 64 == 0 && (size_t)(c_im + i) % 64 == 0) {
            // If the memory is aligned, use aligned store
            _mm512_store_pd(c_re + i, vc_re);
            _mm512_storeu_pd(c_im + i, vc_im);
        }
        else {
            // If the memory is not aligned, use unaligned store
            _mm512_storeu_pd(c_re + i, vc_re);
            _mm512_storeu_pd(c_im + i, vc_im);
        }
    }
#endif

    // Handle the remaining complex numbers if num_complex is not a multiple of 8
    for (; i < num_complex; ++i) {
        double temp_re = a_re[i] * b_re[i] - a_im[i] * b_im[i];
        double temp_im = a_re[i] * b_im[i] + a_im[i] * b_re[i];
        c_re[i] = temp_re;
        c_im[i] = temp_im;
    }
}
