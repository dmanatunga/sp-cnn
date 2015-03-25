#include "CornerDetectionGene.h"

CornerDetectionGene::CornerDetectionGene() 
{
  _A = A_template;
  _B = B_template;
  _z = z_val;
  _r = r_val;
}

const float CornerDetectionGene::A_template[] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float CornerDetectionGene::B_template[] = {-1.0f, -1.0f, -1.0f, -1.0f, 4.0f, -1.0f, -1.0f, -1.0f, -1.0f};
const float CornerDetectionGene::z_val = -5.0f;
const int CornerDetectionGene::r_val = 1;

CNNInput* CornerDetectionGene::getInput(float* in, int R, int C)
{
  CNNInput* ret = new CNNInput(R + 2 * _r, C + 2 * _r);
  float* u = ret->u();
  for (int i = 0; i < R; ++i)
  {
    for (int j = 0; j < C; ++j) {
      u[(i + _r) * (C + 2 * _r) + (j + _r)] = in[i * C + j];
    }
  }
  return ret;
}
