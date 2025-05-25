#include "mlp.h"
#include <Eigen/Dense>
#include <fstream>
#include <vector>

using Eigen::MatrixXf;
using Eigen::VectorXf;

static MatrixXf W1;
static VectorXf b1;
static MatrixXf W2;
static VectorXf b2;

void mlp_init(const char *model_path)
{
    if (!model_path) return;
    std::ifstream f(model_path);
    if (!f)
        return;
    int in, hidden, out;
    if (!(f >> in >> hidden >> out))
        return;
    W1.resize(hidden, in);
    b1.resize(hidden);
    for (int i = 0; i < hidden; ++i)
        for (int j = 0; j < in; ++j)
            f >> W1(i, j);
    for (int i = 0; i < hidden; ++i)
        f >> b1(i);
    W2.resize(out, hidden);
    b2.resize(out);
    for (int i = 0; i < out; ++i)
        for (int j = 0; j < hidden; ++j)
            f >> W2(i, j);
    for (int i = 0; i < out; ++i)
        f >> b2(i);
}

static int input_dim()
{
    return W1.cols() ? W1.cols() : 1;
}

int mlp_predict(const float *features)
{
    Eigen::Map<const VectorXf> x(features, input_dim());
    VectorXf h;
    if (W1.size() == 0)
    {
        h = x;
    }
    else
    {
        h = (W1 * x + b1).array().tanh();
    }
    VectorXf o;
    if (W2.size() == 0)
    {
        o = h;
    }
    else
    {
        o = W2 * h + b2;
    }
    Eigen::Index idx;
    o.maxCoeff(&idx);
    return static_cast<int>(idx);
}
