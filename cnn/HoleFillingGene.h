#ifndef HOLE_FILLING_GENE_H
#define HOLE_FILLING_GENE_H

#include "CNNGene.h"

class HoleFillingGene : public CNNGene {
  private:
       static const float A_template[];
       static const float B_template[];
       static const float z_val;
       static const int r_val;
  
  public:
    HoleFillingGene();
    CNNInput* getInput(float* in, int R, int C);  
};

#endif // HOLE_FILLING_GENE_H
