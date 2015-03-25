#include "CNNGene.h"

// CNN Genes
#include "ConcentricContourGene.h"
#include "ConnectedComponentGene.h"
#include "CornerDetectionGene.h"
#include "EdgeDetectionGene.h"
#include "HoleFillingGene.h"
#include "RotationDetectorGene.h"
#include "ShadowCreatorGene.h"



CNNGene* CNNGeneFactory::createCNNGene(std::string name)
{
  if (name == "ConcentricContour")
    return new ConcentricContourGene();
  else if (name == "ConnectedComponent")
    return new ConnectedComponentGene();
  else if (name == "CornerDetection")
    return new CornerDetectionGene();
  else if (name == "EdgeDetection")
    return new EdgeDetectionGene();
  else if (name == "HoleFilling")
    return new HoleFillingGene();
  else if (name == "RotationDetector")
    return new RotationDetectorGene();
  else if (name == "ShadowCreator")
    return new ShadowCreatorGene();
  else
    // Throw error
  return NULL;
}

