#include "EdgeDetectionGene.h"

EdgeDetectionGene::EdgeDetectionGene() 
{
  _A = A_template;
  _B = B_template;
  _z = z_val;
  _r = r_val;
}

const float EdgeDetectionGene::A_template[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float EdgeDetectionGene::B_template[] = {-1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f};
const float EdgeDetectionGene::z_val = -1.0f;
const int EdgeDetectionGene::r_val = 1;

CNNInput* EdgeDetectionGene::getInput(float* in, int R, int C)
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
