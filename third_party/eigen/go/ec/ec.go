package ec

/*
#cgo CFLAGS: -std=c2x -I${SRCDIR}/../../eigenc/include
#include "ec_core.h"
typedef struct {
    size_t rows;
    size_t cols;
    double *data;
} EC_Matrix2d;
static inline void EC_Matrix2d_mul(const EC_Matrix2d *a, const EC_Matrix2d *b, EC_Matrix2d *out) {
    assert(a->cols == b->rows && a->rows == out->rows && b->cols == out->cols);
    for (size_t i = 0; i < a->rows; ++i) {
        for (size_t j = 0; j < b->cols; ++j) {
            double sum = 0;
            for (size_t k = 0; k < a->cols; ++k)
                sum += a->data[i * a->cols + k] * b->data[k * b->cols + j];
            out->data[i * out->cols + j] = sum;
        }
    }
}
*/
import "C"
import "unsafe"

// MatrixF32 mirrors the C ec_Matrixf32 structure.
type MatrixF32 struct {
	Rows int
	Cols int
	Data []float32
}

// MatrixF64 mirrors the C ec_Matrixf64 structure.
type MatrixF64 struct {
	Rows int
	Cols int
	Data []float64
}

func cMatrixF32(data *C.float, rows, cols int) C.ec_Matrixf32 {
	return C.ec_Matrixf32{
		rows: C.size_t(rows),
		cols: C.size_t(cols),
		data: data,
	}
}

func cMatrixF64(data *C.double, rows, cols int) C.ec_Matrixf64 {
	return C.ec_Matrixf64{
		rows: C.size_t(rows),
		cols: C.size_t(cols),
		data: data,
	}
}

// AddF32 adds two float32 matrices using EigenC.
func AddF32(a, b MatrixF32) MatrixF32 {
	if a.Rows != b.Rows || a.Cols != b.Cols {
		panic("dimension mismatch")
	}
	n := len(a.Data)
	pa := (*C.float)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_float)))
	pb := (*C.float)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_float)))
	pc := (*C.float)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_float)))
	defer C.free(unsafe.Pointer(pa))
	defer C.free(unsafe.Pointer(pb))
	defer C.free(unsafe.Pointer(pc))

	aSlice := (*[1 << 30]C.float)(unsafe.Pointer(pa))[:n:n]
	bSlice := (*[1 << 30]C.float)(unsafe.Pointer(pb))[:n:n]
	for i := 0; i < n; i++ {
		aSlice[i] = C.float(a.Data[i])
		bSlice[i] = C.float(b.Data[i])
	}

	ca := cMatrixF32(pa, a.Rows, a.Cols)
	cb := cMatrixF32(pb, b.Rows, b.Cols)
	co := cMatrixF32(pc, a.Rows, a.Cols)
	C.ec_addf32(&ca, &cb, &co)

	out := MatrixF32{Rows: a.Rows, Cols: a.Cols, Data: make([]float32, n)}
	cSlice := (*[1 << 30]C.float)(unsafe.Pointer(pc))[:n:n]
	for i := 0; i < n; i++ {
		out.Data[i] = float32(cSlice[i])
	}
	return out
}

// AddF64 adds two float64 matrices using EigenC.
func AddF64(a, b MatrixF64) MatrixF64 {
	if a.Rows != b.Rows || a.Cols != b.Cols {
		panic("dimension mismatch")
	}
	n := len(a.Data)
	pa := (*C.double)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_double)))
	pb := (*C.double)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_double)))
	pc := (*C.double)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_double)))
	defer C.free(unsafe.Pointer(pa))
	defer C.free(unsafe.Pointer(pb))
	defer C.free(unsafe.Pointer(pc))

	aSlice := (*[1 << 30]C.double)(unsafe.Pointer(pa))[:n:n]
	bSlice := (*[1 << 30]C.double)(unsafe.Pointer(pb))[:n:n]
	for i := 0; i < n; i++ {
		aSlice[i] = C.double(a.Data[i])
		bSlice[i] = C.double(b.Data[i])
	}

	ca := cMatrixF64(pa, a.Rows, a.Cols)
	cb := cMatrixF64(pb, b.Rows, b.Cols)
	co := cMatrixF64(pc, a.Rows, a.Cols)
	C.ec_addf64(&ca, &cb, &co)

	out := MatrixF64{Rows: a.Rows, Cols: a.Cols, Data: make([]float64, n)}
	cSlice := (*[1 << 30]C.double)(unsafe.Pointer(pc))[:n:n]
	for i := 0; i < n; i++ {
		out.Data[i] = float64(cSlice[i])
	}
	return out
}

// MulF32 multiplies two 2x2 float32 matrices using EigenC.
func MulF32(a, b MatrixF32) MatrixF32 {
	if a.Rows != 2 || a.Cols != 2 || b.Rows != 2 || b.Cols != 2 {
		panic("MulF32 expects 2x2 matrices")
	}
	n := 4
	pa := (*C.float)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_float)))
	pb := (*C.float)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_float)))
	pc := (*C.float)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_float)))
	defer C.free(unsafe.Pointer(pa))
	defer C.free(unsafe.Pointer(pb))
	defer C.free(unsafe.Pointer(pc))

	aSlice := (*[4]C.float)(unsafe.Pointer(pa))[:]
	bSlice := (*[4]C.float)(unsafe.Pointer(pb))[:]
	for i := 0; i < n; i++ {
		aSlice[i] = C.float(a.Data[i])
		bSlice[i] = C.float(b.Data[i])
	}

	ca := C.EC_Matrix2f{rows: 2, cols: 2, data: pa}
	cb := C.EC_Matrix2f{rows: 2, cols: 2, data: pb}
	co := C.EC_Matrix2f{rows: 2, cols: 2, data: pc}
	C.EC_Matrix2f_mul(&ca, &cb, &co)

	out := MatrixF32{Rows: 2, Cols: 2, Data: make([]float32, n)}
	cSlice := (*[4]C.float)(unsafe.Pointer(pc))[:]
	for i := 0; i < n; i++ {
		out.Data[i] = float32(cSlice[i])
	}
	return out
}

// MulF64 multiplies two 2x2 float64 matrices using EigenC.
func MulF64(a, b MatrixF64) MatrixF64 {
	if a.Rows != 2 || a.Cols != 2 || b.Rows != 2 || b.Cols != 2 {
		panic("MulF64 expects 2x2 matrices")
	}
	n := 4
	pa := (*C.double)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_double)))
	pb := (*C.double)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_double)))
	pc := (*C.double)(C.malloc(C.size_t(n) * C.size_t(C.sizeof_double)))
	defer C.free(unsafe.Pointer(pa))
	defer C.free(unsafe.Pointer(pb))
	defer C.free(unsafe.Pointer(pc))

	aSlice := (*[4]C.double)(unsafe.Pointer(pa))[:]
	bSlice := (*[4]C.double)(unsafe.Pointer(pb))[:]
	for i := 0; i < n; i++ {
		aSlice[i] = C.double(a.Data[i])
		bSlice[i] = C.double(b.Data[i])
	}

	ca := C.EC_Matrix2d{rows: 2, cols: 2, data: pa}
	cb := C.EC_Matrix2d{rows: 2, cols: 2, data: pb}
	co := C.EC_Matrix2d{rows: 2, cols: 2, data: pc}
	C.EC_Matrix2d_mul(&ca, &cb, &co)

	out := MatrixF64{Rows: 2, Cols: 2, Data: make([]float64, n)}
	cSlice := (*[4]C.double)(unsafe.Pointer(pc))[:]
	for i := 0; i < n; i++ {
		out.Data[i] = float64(cSlice[i])
	}
	return out
}
