#include "ConnectedComponentGene.h"

ConnectedComponentGene::ConnectedComponentGene() 
{
  _A = A_template;
  _B = B_template;
  _z = z_val;
  _r = r_val;
}

const float ConnectedComponentGene::A_template[] = {0.0f, 0.0f, 0.0f, 1.0f, 2.0f, -1.0f, 0.0f, 0.0f, 0.0f};
const float ConnectedComponentGene::B_template[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float ConnectedComponentGene::z_val = 0.0f;
const int ConnectedComponentGene::r_val = 1;

CNNInput* ConnectedComponentGene::getInput(float* in, int R, int C)
{
  CNNInput* ret = new CNNInput(R + 2 * _r, C + 2 * _r);
  float* initState = ret->initialState();
  for (int i = 0; i < R; ++i)
  {
    for (int j = 0; j < C; ++j) {
      initState[(i + _r) * (C + 2 * _r) + (j + _r)] = in[i * C + j];
    }
  }
  return ret;
}
