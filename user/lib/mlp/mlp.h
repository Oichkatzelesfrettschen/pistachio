#ifndef LIB_MLP_H
#define LIB_MLP_H
#ifdef __cplusplus
extern "C" {
#endif

void mlp_init(const char *model_path);
int mlp_predict(const float *features);

#ifdef __cplusplus
}
#endif
#endif // LIB_MLP_H
