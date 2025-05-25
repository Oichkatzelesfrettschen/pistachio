#include <fstream>
#include "ec_generated.h"

int main() {
    float A[4] = {1,2,3,4};
    float B[4] = {5,6,7,8};
    float C_add[4];
    float C_mul[4];

    for(int i=0;i<4;i++)
        C_add[i] = A[i] + B[i];

    EC_Matrix2f Ac = {2,2,A};
    EC_Matrix2f Bc = {2,2,B};
    EC_Matrix2f Cc = {2,2,C_mul};
    EC_Matrix2f_mul(&Ac, &Bc, &Cc);

    std::ofstream f("/tmp/cpp_out.txt");
    for(int i=0;i<4;i++)
        f << C_add[i] << "\n";
    for(int i=0;i<4;i++)
        f << C_mul[i] << "\n";
    return 0;
}
