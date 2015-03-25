#include "CNNGene.h"

CNNInput::CNNInput(int R, int C)
{
  _R = R;
  _C = C;
  _initialState = new float[_R * _C]();
  _u = new float[_R * _C]();
}

CNNInput::~CNNInput()
{
  delete[] _initialState;
  delete[] _u;
}
