#ifndef EDGE_DETECTION_GENE_H
#define EDGE_DETECTION_GENE_H

#include "CNNGene.h"

class EdgeDetectionGene : public CNNGene {
  private:
       static const float A_template[];
       static const float B_template[];
       static const float z_val;
       static const int r_val;
  
  public:
    EdgeDetectionGene();
    CNNInput* getInput(float* in, int R, int C);  
};

#endif // EDGE_DETECTION_GENE_H
