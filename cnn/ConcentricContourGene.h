#ifndef CONCENTRIC_CONTOUR_GENE_H
#define CONCENTRIC_CONTOUR_GENE_H

#include "CNNGene.h"

class ConcentricContourGene : public CNNGene {
  private:
       static const float A_template[];
       static const float B_template[];
       static const float z_val;
       static const int r_val;
  
  public:
    ConcentricContourGene();
    CNNInput* getInput(float* in, int R, int C);  
};

#endif // CONCENTRIC_CONTOUR_GENE_H
