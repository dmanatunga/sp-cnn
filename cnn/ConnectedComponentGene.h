#ifndef CONNECTED_COMPONENT_GENE_H
#define CONNECTED_COMPONENT_GENE_H

#include "CNNGene.h"

class ConnectedComponentGene : public CNNGene {
  private:
       static const float A_template[];
       static const float B_template[];
       static const float z_val;
       static const int r_val;
  
  public:
    ConnectedComponentGene();
    CNNInput* getInput(float* in, int R, int C);  
};

#endif // CONNECTED_COMPONENT_GENE_H
