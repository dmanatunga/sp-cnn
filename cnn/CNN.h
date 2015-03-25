#ifndef CNN_H
#define CNN_H

#include<vector>
#include<stdlib.h>

#include "CNNInput.h"
#include "CNNGene.h"

class CNN {
protected:
	CNNGene*_gene;
	CNNInput* _input;

	size_t _M;
	size_t _N;
	size_t _r;

public:
	virtual void run(CNNGene* gene, CNNInput *input, unsigned int N, float dt) = 0;
	virtual void run(CNNGene* gene, CNNInput *input, unsigned int N) = 0;

	virtual unsigned int convergenceRun(CNNGene* gene, CNNInput *input, float dt) = 0;
	virtual unsigned int convergenceRun(CNNGene* gene, CNNInput *input) = 0;
	virtual std::vector<long long int> outputAnalysisRun(CNNGene* gene, CNNInput* input, float* idealOutput) = 0;

	virtual void copyOutput(float* out) = 0;

	CNNGene* gene() { return _gene; }
	CNNInput* input() { return _input; }
	size_t M() { return _M; }
	size_t N() { return _N; }
	size_t r() { return _r; }
};

#endif // CNN_H
