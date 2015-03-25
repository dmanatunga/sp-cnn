#include "RotationDetectorGene.h"

RotationDetectorGene::RotationDetectorGene() 
{
  _A = A_template;
  _B = B_template;
  _z = z_val;
  _r = r_val;
}

const float RotationDetectorGene::A_template[] = {-0.8f, 5.0f, -0.8f, 5.0f, 5.0f, 5.0f, -0.8f, 5.0f, -0.8f};
const float RotationDetectorGene::B_template[] = {-0.4f, -2.5f, -0.4f, -2.5f, 5.0f, -2.5f, -0.4f, -2.5f, -0.4f};
const float RotationDetectorGene::z_val = 11.2f;
const int RotationDetectorGene::r_val = 1;

CNNInput* RotationDetectorGene::getInput(float* in, int R, int C)
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
