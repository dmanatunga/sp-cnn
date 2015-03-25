#ifndef CORNER_DETECTION_GENE_H
#define CORNER_DETECTION_GENE_H

#include "CNNGene.h"

class CornerDetectionGene : public CNNGene {
  private:
       static const float A_template[];
       static const float B_template[];
       static const float z_val;
       static const int r_val;
  
  public:
    CornerDetectionGene();
    CNNInput* getInput(float* in, int R, int C);  
};

#endif // CORNER_DETECTION_GENE_H
