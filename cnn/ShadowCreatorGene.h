#ifndef SHADOW_CREATOR_GENE_H
#define SHADOW_CREATOR_GENE_H

#include "CNNGene.h"

class ShadowCreatorGene : public CNNGene {
  private:
       static const float A_template[];
       static const float B_template[];
       static const float z_val;
       static const int r_val;
  
  public:
    ShadowCreatorGene();
    CNNInput* getInput(float* in, int R, int C);  
};

#endif // SHADOW_CREATOR_GENE_H
