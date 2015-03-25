#ifndef ROTATION_DETECTOR_GENE_H
#define ROTATION_DETECTOR_GENE_H

#include "CNNGene.h"

class RotationDetectorGene : public CNNGene {
  private:
       static const float A_template[];
       static const float B_template[];
       static const float z_val;
       static const int r_val;
  
  public:
    RotationDetectorGene();
    CNNInput* getInput(float* in, int R, int C);  
};

#endif // ROTATION_DETECTOR_GENE_H
