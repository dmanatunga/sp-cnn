#ifndef CNN_GENE_H
#define CNN_GENE_H

#include <string>

#include "CNNInput.h"

class CNNGene {
  protected:
    const float* _A;
    const float* _B;
    float _z;
    int _r;

  public:
    const float* A() { return _A; }
    const float* B() { return _B; }
    float z() { return _z; }
    int r() { return _r; }

    virtual CNNInput* getInput(float* in, int R, int C) = 0;  
};

class CNNGeneFactory {
  public:
    static CNNGene* createCNNGene(std::string name);
};

#endif // CNN_GENE_H

