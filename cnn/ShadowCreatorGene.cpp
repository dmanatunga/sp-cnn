#include "ShadowCreatorGene.h"

ShadowCreatorGene::ShadowCreatorGene() 
{
  _A = A_template;
  _B = B_template;
  _z = z_val;
  _r = r_val;
}

const float ShadowCreatorGene::A_template[] = {0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f};
const float ShadowCreatorGene::B_template[] = {0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float ShadowCreatorGene::z_val = 0.0f;
const int ShadowCreatorGene::r_val = 1;

CNNInput* ShadowCreatorGene::getInput(float* in, int R, int C)
{
  CNNInput* ret = new CNNInput(R + 2 * _r, C + 2 * _r);
  float* initState = ret->initialState();
  float* u = ret->u();
  for (int i = 0; i < R; ++i)
  {
    for (int j = 0; j < C; ++j) {
      initState[(i + _r) * (C + 2 * _r) + (j + _r)] = 1.0f;
      u[(i + _r) * (C + 2 * _r) + (j + _r)] = in[i * C + j];
    }
  }
  return ret;
}
