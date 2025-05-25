package ec

import "fmt"

func ExampleAddF32() {
	a := MatrixF32{Rows: 2, Cols: 2, Data: []float32{1, 2, 3, 4}}
	b := MatrixF32{Rows: 2, Cols: 2, Data: []float32{5, 6, 7, 8}}
	c := AddF32(a, b)
	fmt.Println(c.Data)
	// Output: [6 8 10 12]
}
