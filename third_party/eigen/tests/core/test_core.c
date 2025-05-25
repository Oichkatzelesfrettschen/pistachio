#include "ec_core.h"
#include "ec_generated.h"
#include <stdio.h>

int main(void) {
    float a_data[4] = {1,2,3,4};
    float b_data[4] = {5,6,7,8};
    float add_data[4];
    float mul_data[4];

    ec_Matrixf32 A = {2,2,a_data};
    ec_Matrixf32 B = {2,2,b_data};
    ec_Matrixf32 C_add = {2,2,add_data};
    ec_addf32(&A, &B, &C_add);

    EC_Matrix2f Ag = {2,2,a_data};
    EC_Matrix2f Bg = {2,2,b_data};
    EC_Matrix2f C_mul = {2,2,mul_data};
    EC_Matrix2f_mul(&Ag, &Bg, &C_mul);

    ec_DMatrixf32 Da = ec_dmatrixf32_alloc(2,2);
    ec_DMatrixf32 Db = ec_dmatrixf32_alloc(2,2);
    ec_DMatrixf32 Dc = ec_dmatrixf32_alloc(2,2);
    for (size_t i=0;i<4;++i){
        Da.data[i]=a_data[i];
        Db.data[i]=b_data[i];
    }
    ec_addf32((const ec_Matrixf32*)&Da, (const ec_Matrixf32*)&Db, (ec_Matrixf32*)&Dc);
    for (size_t i=0;i<4;++i)
        add_data[i] = Dc.data[i];
    ec_dmatrixf32_free(&Da);
    ec_dmatrixf32_free(&Db);
    ec_dmatrixf32_free(&Dc);

    FILE *f = fopen("/tmp/c_out.txt", "w");
    for (size_t i=0; i<4; ++i) fprintf(f, "%f\n", add_data[i]);
    for (size_t i=0; i<4; ++i) fprintf(f, "%f\n", mul_data[i]);
    fclose(f);
    return 0;
}
