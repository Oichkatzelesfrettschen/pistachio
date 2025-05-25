#ifndef EC_CORE_H
#define EC_CORE_H
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include "ec_generated.h"
#if defined(__AVX__) || defined(__SSE__)
#  include <immintrin.h>
#endif
#if defined(__PPU__) || defined(__POWERPC__)
#  include <altivec.h>
#endif

typedef struct {
    size_t rows;
    size_t cols;
    float *data;
} ec_Matrixf32;

typedef struct {
    size_t rows;
    size_t cols;
    double *data;
} ec_Matrixf64;

typedef struct {
    size_t rows;
    size_t cols;
    float *data;
} ec_DMatrixf32;

typedef struct {
    size_t rows;
    size_t cols;
    double *data;
} ec_DMatrixf64;

static inline ec_DMatrixf32 ec_dmatrixf32_alloc(size_t rows, size_t cols) {
    ec_DMatrixf32 m;
    m.rows = rows;
    m.cols = cols;
    m.data = (float*)malloc(rows * cols * sizeof(float));
    return m;
}

static inline void ec_dmatrixf32_free(ec_DMatrixf32 *m) {
    free(m->data);
    m->data = NULL;
    m->rows = m->cols = 0;
}

static inline ec_DMatrixf64 ec_dmatrixf64_alloc(size_t rows, size_t cols) {
    ec_DMatrixf64 m;
    m.rows = rows;
    m.cols = cols;
    m.data = (double*)malloc(rows * cols * sizeof(double));
    return m;
}

static inline void ec_dmatrixf64_free(ec_DMatrixf64 *m) {
    free(m->data);
    m->data = NULL;
    m->rows = m->cols = 0;
}

#ifndef EC_RESTRICT
#  ifdef __cplusplus
#    define EC_RESTRICT
#  else
#    define EC_RESTRICT restrict
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline void ec_addf32(const ec_Matrixf32 *a, const ec_Matrixf32 *b, ec_Matrixf32 *out) {
    assert(a->rows == b->rows && a->cols == b->cols && a->rows == out->rows && a->cols == out->cols);
    size_t n = a->rows * a->cols;
#if defined(__AVX__)
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(a->data + i);
        __m256 vb = _mm256_loadu_ps(b->data + i);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(out->data + i, vc);
    }
    for (; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#elif defined(__SSE__)
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 va = _mm_loadu_ps(a->data + i);
        __m128 vb = _mm_loadu_ps(b->data + i);
        __m128 vc = _mm_add_ps(va, vb);
        _mm_storeu_ps(out->data + i, vc);
    }
    for (; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#elif defined(__PPU__) || defined(__POWERPC__)
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        vector float va = vec_ld(0, a->data + i);
        vector float vb = vec_ld(0, b->data + i);
        vector float vc = vec_add(va, vb);
        vec_st(vc, 0, out->data + i);
    }
    for (; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#else
    for (size_t i = 0; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#endif
}

static inline void ec_addf64(const ec_Matrixf64 *a, const ec_Matrixf64 *b, ec_Matrixf64 *out) {
    assert(a->rows == b->rows && a->cols == b->cols && a->rows == out->rows && a->cols == out->cols);
    size_t n = a->rows * a->cols;
#if defined(__AVX__)
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m256d va = _mm256_loadu_pd(a->data + i);
        __m256d vb = _mm256_loadu_pd(b->data + i);
        __m256d vc = _mm256_add_pd(va, vb);
        _mm256_storeu_pd(out->data + i, vc);
    }
    for (; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#elif defined(__SSE__)
    size_t i = 0;
    for (; i + 2 <= n; i += 2) {
        __m128d va = _mm_loadu_pd(a->data + i);
        __m128d vb = _mm_loadu_pd(b->data + i);
        __m128d vc = _mm_add_pd(va, vb);
        _mm_storeu_pd(out->data + i, vc);
    }
    for (; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#elif defined(__PPU__) || defined(__POWERPC__)
    size_t i = 0;
    for (; i + 2 <= n; i += 2) {
        vector double va = vec_ld(0, a->data + i);
        vector double vb = vec_ld(0, b->data + i);
        vector double vc = vec_add(va, vb);
        vec_st(vc, 0, out->data + i);
    }
    for (; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#else
    for (size_t i = 0; i < n; ++i)
        out->data[i] = a->data[i] + b->data[i];
#endif
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EC_CORE_H */
