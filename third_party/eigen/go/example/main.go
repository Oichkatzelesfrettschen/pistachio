package main

import (
	"ec/ec"
	"fmt"
)

func main() {
	a := ec.MatrixF32{Rows: 2, Cols: 2, Data: []float32{1, 2, 3, 4}}
	b := ec.MatrixF32{Rows: 2, Cols: 2, Data: []float32{5, 6, 7, 8}}
	c := ec.AddF32(a, b)
	fmt.Println(c.Data)
}
