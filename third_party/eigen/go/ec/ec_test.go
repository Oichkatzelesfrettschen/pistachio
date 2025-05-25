package ec

import "testing"

func TestAddF32(t *testing.T) {
	a := MatrixF32{Rows: 2, Cols: 2, Data: []float32{1, 2, 3, 4}}
	b := MatrixF32{Rows: 2, Cols: 2, Data: []float32{5, 6, 7, 8}}
	c := AddF32(a, b)
	expected := []float32{6, 8, 10, 12}
	for i, v := range expected {
		if c.Data[i] != v {
			t.Fatalf("c[%d] = %f, want %f", i, c.Data[i], v)
		}
	}
}

func TestAddF64(t *testing.T) {
	a := MatrixF64{Rows: 2, Cols: 2, Data: []float64{1, 2, 3, 4}}
	b := MatrixF64{Rows: 2, Cols: 2, Data: []float64{5, 6, 7, 8}}
	c := AddF64(a, b)
	expected := []float64{6, 8, 10, 12}
	for i, v := range expected {
		if c.Data[i] != v {
			t.Fatalf("c[%d] = %f, want %f", i, c.Data[i], v)
		}
	}
}

func TestMulF32(t *testing.T) {
	a := MatrixF32{Rows: 2, Cols: 2, Data: []float32{1, 2, 3, 4}}
	b := MatrixF32{Rows: 2, Cols: 2, Data: []float32{5, 6, 7, 8}}
	c := MulF32(a, b)
	expected := []float32{19, 22, 43, 50}
	for i, v := range expected {
		if c.Data[i] != v {
			t.Fatalf("c[%d] = %f, want %f", i, c.Data[i], v)
		}
	}
}

func TestMulF64(t *testing.T) {
	a := MatrixF64{Rows: 2, Cols: 2, Data: []float64{1, 2, 3, 4}}
	b := MatrixF64{Rows: 2, Cols: 2, Data: []float64{5, 6, 7, 8}}
	c := MulF64(a, b)
	expected := []float64{19, 22, 43, 50}
	for i, v := range expected {
		if c.Data[i] != v {
			t.Fatalf("c[%d] = %f, want %f", i, c.Data[i], v)
		}
	}
}
