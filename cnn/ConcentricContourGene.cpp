#include "ConcentricContourGene.h"

ConcentricContourGene::ConcentricContourGene() 
{
  _A = A_template;
  _B = B_template;
  _z = z_val;
  _r = r_val;
}

const float ConcentricContourGene::A_template[] = {0.0f, -1.0f, 0.0f, -1.0f, 3.5f, -1.0f, 0.0f, -1.0f, 0.0f};
const float ConcentricContourGene::B_template[] = {0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float ConcentricContourGene::z_val = -4.0f;
const int ConcentricContourGene::r_val = 1;

CNNInput* ConcentricContourGene::getInput(float* in, int R, int C)
{
  CNNInput* ret = new CNNInput(R + 2 * _r, C + 2 * _r);
  float* initState = ret->initialState();
  float* u = ret->u();
  for (int i = 0; i < R; ++i)
  {
    for (int j = 0; j < C; ++j) {
      initState[(i + _r) * (C + 2 * _r) + (j + _r)] = in[i * C + j];
      u[(i + _r) * (C + 2 * _r) + (j + _r)] = in[i * C + j];
    }
  }
  return ret;
}
