#ifndef EC_GENERATED_H
#define EC_GENERATED_H
#include <stddef.h>
#include <assert.h>

typedef struct {
    size_t rows;
    size_t cols;
    float *data;
} EC_Matrix2f;

static inline void EC_Matrix2f_add(const EC_Matrix2f *a, const EC_Matrix2f *b, EC_Matrix2f *out) {
    assert(a->rows == b->rows && a->cols == b->cols && a->rows == out->rows && a->cols == out->cols);
    for (size_t i = 0; i < a->rows * a->cols; ++i)
        out->data[i] = a->data[i] + b->data[i];
}

static inline void EC_Matrix2f_mul(const EC_Matrix2f *a, const EC_Matrix2f *b, EC_Matrix2f *out) {
    assert(a->cols == b->rows && a->rows == out->rows && b->cols == out->cols);
    for (size_t i = 0; i < a->rows; ++i) {
        for (size_t j = 0; j < b->cols; ++j) {
            float sum = 0;
            for (size_t k = 0; k < a->cols; ++k)
                sum += a->data[i * a->cols + k] * b->data[k * b->cols + j];
            out->data[i * out->cols + j] = sum;
        }
    }
}

#define ec_add(A,B,OUT) \
    _Generic((A), \
        EC_Matrix2f*: EC_Matrix2f_add, \
        const EC_Matrix2f*: EC_Matrix2f_add \
    )(A,B,OUT)

#define ec_mul(A,B,OUT) \
    _Generic((A), \
        EC_Matrix2f*: EC_Matrix2f_mul, \
        const EC_Matrix2f*: EC_Matrix2f_mul \
    )(A,B,OUT)

#endif /* EC_GENERATED_H */
