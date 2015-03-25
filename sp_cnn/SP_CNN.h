/*
 * SP_CNN_CPU.h
 *
 *  Created on: Apr 10, 2014
 *      Author: manatunga
 */

#ifndef SP_CNN_H_
#define SP_CNN_H_

#include <stdlib.h>
#include "PartitionStream.h"
#include "cnn/cnn.h"

class CNN_P {
private:
	size_t _M;
	size_t _N;
	size_t _r;

	CNNGene* _gene;
	float* _state;
	float* _output;
	float* _u;

	size_t _R;
	size_t _C;

public:
	CNN_P(size_t M, size_t N, size_t _r);
	~CNN_P();

	bool run(unsigned int N);
	unsigned int convergenceRun(unsigned int N);
	unsigned int convergenceRun();
	float* state() { return _state; }
	float* u() { return _u; }

	void setGene(CNNGene* gene) { _gene = gene; }
private:
	bool computeNextState();
	void computeInitialOutput();
	void computeOutput();
};

class SP_CNN {
public:
	class RunData {
	public:
		unsigned int numIters;
		unsigned int totalConvTime;
		unsigned int virtualConvTime;
		std::vector<unsigned int> cnnP_convTime;
		std::vector<unsigned int> partition_convTime;
		std::vector<unsigned int> virtualConvTime_perIter;


	public:
		RunData() {
			numIters = 0;
			totalConvTime = 0;
			virtualConvTime = 0;
		}
		RunData(unsigned int n, unsigned int totConv, unsigned int virtConv) {
			numIters = n;
			totalConvTime = totConv;
			virtualConvTime = virtConv;
		}
	};

private:
	size_t _M;
	size_t _N;
	size_t _r;

	size_t _num_CNN_P;
	size_t _cnnM;
	size_t _cnnN;

	size_t _R;
	size_t _C;

	float* _state;
	float* _nextState;
	float* _output;

	std::vector<CNN_P*> _CNN_P_units;


public:
	SP_CNN(size_t M, size_t N, size_t r, size_t cnnM, size_t cnnN, unsigned char num_CNN_P);
	~SP_CNN();

	RunData fixedIntervalRun(CNNGene* gene, CNNInput* input,
			 unsigned int interval_N, std::string partition_order);

	RunData fixedIntervalRunFastProp(CNNGene* gene, CNNInput* input,
				 unsigned int interval_N, std::string partition_order);

	RunData fixedIntervalRunConvOpt(CNNGene* gene, CNNInput* input,
			 unsigned int interval_N, std::string partition_order);

	RunData earlyFinishIntervalRun(CNNGene* gene, CNNInput* input,
				 unsigned int interval_N, std::string partition_order);

	RunData earlyFinishIntervalRunFastProp(CNNGene* gene, CNNInput* input,
					 unsigned int interval_N, std::string partition_order);

	RunData earlyFinishIntervalRunConvOpt(CNNGene* gene, CNNInput* input,
					 unsigned int interval_N, std::string partition_order);

	RunData naiveRunNoShareBoundary(CNNGene* gene, float* image);
	RunData naiveRun(CNNGene* gene, CNNInput* input);

	void copyOutput(float* out);

private:
	void computeOutput(float* state);
	void initialize_CNN_P(Partition p, CNN_P* cnnUnit, const float* u);
	void copy_CNN_P_state(Partition p, CNN_P* cnnUnit);
};




#endif /* SP_CNN_H_ */
