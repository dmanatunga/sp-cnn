#include "SP_CNN.h"
#include <cmath>
#include <string.h>
#include <assert.h>
#include <iostream>

#include "PartitionGenerator.h"
#include "cnn/cnn.h"

#define CHANGE_THRESHOLD 0.00001
#define COMPARE_THRESHOLD 0.00001

#define SP_CNN_RUN_ITERATION_LIMIT 10000
#define CNN_CONVERGENCE_LIMIT 5000

//////////////////////////////////////////////////////////////
// SP_CNN
//////////////////////////////////////////////////////////////
SP_CNN::SP_CNN(size_t M, size_t N, size_t r, size_t cnnM, size_t cnnN, uint8_t num_CNN_P)
{
	_M = M;
	_N = N;
	_r = r;

	_R = M + 2 * r;
	_C = N + 2 * r;


	_num_CNN_P = num_CNN_P;
	_cnnM = cnnM;
	_cnnN = cnnN;
	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units.push_back(new CNN_P(_cnnM, _cnnN, _r));
	}


	_state = new float[_R * _C];
	_nextState = new float[_R * _C];
	_output = new float[_R * _C];
}

SP_CNN::~SP_CNN()
{
	for (int i = 0; i < _num_CNN_P; i++) {
		delete _CNN_P_units[i];
	}
	delete[] _state;
	delete[] _nextState;
	delete[] _output;
}

SP_CNN::RunData SP_CNN::fixedIntervalRun(CNNGene* gene, CNNInput* input, unsigned int interval_N, std::string partition_order)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);

	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
	}


	PartitionStream stream = get_partition_stream(partition_order, _M, _N, _cnnM, _cnnN);

	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	SP_CNN::RunData runData;

	int curCNN_P = 0;
	for (unsigned int i = 1; i <= SP_CNN_RUN_ITERATION_LIMIT; i++) {
		bool change = false;
		for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
			Partition p = stream.getPartition(j);
			CNN_P* cnnUnit = _CNN_P_units[curCNN_P];
			initialize_CNN_P(p, cnnUnit, u);
			change |= cnnUnit->run(interval_N);
			copy_CNN_P_state(p, cnnUnit);
			if (++curCNN_P == _num_CNN_P) {
				curCNN_P = 0;
				runData.totalConvTime += interval_N;
			}
		}
		if (curCNN_P != 0) {
			curCNN_P = 0;
			runData.totalConvTime += interval_N;
		}
		runData.virtualConvTime += interval_N;
		float* tmp = _nextState;
		_nextState = _state;
		_state = tmp;
		this->computeOutput(_state);

		if (!change) {
			runData.numIters = i;
			return runData;
		}
	}
	std::cout << "ERROR: Reached iteration limit" << std::endl;
	return runData;
}

SP_CNN::RunData SP_CNN::fixedIntervalRunFastProp(CNNGene* gene, CNNInput* input, unsigned int interval_N, std::string partition_order)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);
	assert(_num_CNN_P == 1);

	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
	}


	PartitionStream stream = get_partition_stream(partition_order, _M, _N, _cnnM, _cnnN);

	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	SP_CNN::RunData runData;

	int curCNN_P = 0;
	for (unsigned int n = 1; n <= SP_CNN_RUN_ITERATION_LIMIT; n++) {
		bool change = false;
		for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
			Partition p = stream.getPartition(j);
			CNN_P* cnnUnit = _CNN_P_units[curCNN_P];
			initialize_CNN_P(p, cnnUnit, u);
			change |= cnnUnit->run(interval_N);

			float* cnnState = cnnUnit->state();
			size_t indR = p.indR1() + _r;
			size_t indC = p.indC1() + _r;
			for (size_t i = 0; i < _cnnM; ++i) {
				memcpy(&_state[(indR + i) * _C + indC], &cnnState[(i + _r) * (_cnnN + 2 * _r) + _r], sizeof(float) * _cnnN);
			}

			if (++curCNN_P == _num_CNN_P) {
				curCNN_P = 0;
				runData.totalConvTime += interval_N;
			}
		}
		if (curCNN_P != 0) {
			curCNN_P = 0;
			runData.totalConvTime += interval_N;
		}
		runData.virtualConvTime += interval_N;
		this->computeOutput(_state);

		if (!change) {
			runData.numIters = n;
			return runData;
		}
	}
	std::cout << "ERROR: Reached iteration limit" << std::endl;
	return runData;
}

SP_CNN::RunData SP_CNN::fixedIntervalRunConvOpt(CNNGene* gene, CNNInput* input, unsigned int interval_N, std::string partition_order)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);

	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
	}


	PartitionStream stream = get_partition_stream(partition_order, _M, _N, _cnnM, _cnnN);

	bool partitionChanged[stream.dimX()+2][stream.dimY()+2];
	bool oldPartitionChanged[stream.dimX()+2][stream.dimY()+2];

	for (unsigned int i = 0; i < stream.dimX() + 2; i++) {
		for (unsigned int j = 0; j < stream.dimY() + 2; j++) {
			if ((i == 0) || (i == (stream.dimX() + 1)) ||
					(j == 0) || (j == (stream.dimY() + 1))) {
				partitionChanged[i][j] = false;
			} else {
				partitionChanged[i][j] = true;
			}
		}
	}

	for (unsigned int i = 0; i < stream.dimX() + 2; i++) {
		for (unsigned int j = 0; j < stream.dimY() + 2; j++) {
			oldPartitionChanged[i][j] = partitionChanged[i][j];
		}
	}
	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	SP_CNN::RunData runData;

	int curCNN_P = 0;
	for (unsigned int i = 1; i <= SP_CNN_RUN_ITERATION_LIMIT; i++) {
		bool change = false;

		unsigned int tmp_interval_time = 0;
		for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
			Partition p = stream.getPartition(j);

			bool canSkip = true;
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					canSkip &= ~oldPartitionChanged[p.idX() + 1 + i][p.idY() + 1 + j];
				}
			}

			if (canSkip) {
				//std::cout << "P(" << p.idX() << "," << p.idY() << "): " << " skip" << std::endl;
				continue;
			}

			tmp_interval_time = interval_N;

			CNN_P* cnnUnit = _CNN_P_units[curCNN_P];
			initialize_CNN_P(p, cnnUnit, u);
			bool p_changed = cnnUnit->run(interval_N);
			//std::cout << "P(" << p.idX() << "," << p.idY() << "): " << (p_changed ? "true" : "false") << std::endl;


			partitionChanged[p.idX() + 1][p.idY() + 1] = p_changed;
			change |= p_changed;

			copy_CNN_P_state(p, cnnUnit);
			if (++curCNN_P == _num_CNN_P) {
				curCNN_P = 0;
				runData.totalConvTime += tmp_interval_time;
			}
		}

		if (curCNN_P != 0) {
			curCNN_P = 0;
			runData.totalConvTime += tmp_interval_time;
		}
		runData.virtualConvTime += tmp_interval_time;

		for (unsigned int i = 0; i < stream.dimX() + 2; i++) {
			for (unsigned int j = 0; j < stream.dimY() + 2; j++) {
				oldPartitionChanged[i][j] = partitionChanged[i][j];
			}
		}

		float* tmp = _nextState;
		_nextState = _state;
		_state = tmp;
		this->computeOutput(_state);

		if (!change) {
			runData.numIters = i;
			return runData;
		}
	}
	std::cout << "ERROR: Reached iteration limit" << std::endl;
	return runData;
}

SP_CNN::RunData SP_CNN::earlyFinishIntervalRun(CNNGene* gene, CNNInput* input, unsigned int interval_N, std::string partition_order)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);


	unsigned int unit_conv_times[_num_CNN_P];
	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
		unit_conv_times[i] = 0;
	}


	PartitionStream stream = get_partition_stream(partition_order, _M, _N, _cnnM, _cnnN);
	unsigned int partition_conv_times[stream.numPartitions()];
	for (unsigned int i = 0; i < stream.numPartitions(); i++) {
		partition_conv_times[i] = 0;
	}

	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	SP_CNN::RunData runData;

	unsigned int virtual_conv_time = 0;
	for (unsigned int i = 1; i <= SP_CNN_RUN_ITERATION_LIMIT; i++) {
		bool change = false;
		unsigned int intervalVirtTime = 0;
		for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
			Partition p = stream.getPartition(j);
			int curCNN_P = 0;
			int curCNNval = unit_conv_times[curCNN_P];
			for (unsigned int k = 1; k < _num_CNN_P; k++) {
				if (unit_conv_times[k] < curCNNval) {
					curCNN_P = k;
					curCNNval = unit_conv_times[k];
				}
			}
			CNN_P* cnnUnit = _CNN_P_units[curCNN_P];
			initialize_CNN_P(p, cnnUnit, u);
			unsigned int convTime = cnnUnit->convergenceRun(interval_N);
			copy_CNN_P_state(p, cnnUnit);

			change |= (convTime > 1) ? true : false;

			if (convTime > intervalVirtTime) {
				intervalVirtTime = convTime;
			}
			unit_conv_times[curCNN_P] += convTime;
			partition_conv_times[j] += convTime;
		}


		int curCNNval = 0;
		// One partition finished processing, so set current
		// completion to time of last executing partition.
		for (unsigned int k = 0; k < _num_CNN_P; k++) {
			if (unit_conv_times[k] > curCNNval) {
				curCNNval = unit_conv_times[k];
			}
		}

		for (unsigned int k = 0; k < _num_CNN_P; k++) {
			if (unit_conv_times[k] > curCNNval) {
				unit_conv_times[k] = curCNNval;
			}
		}

		virtual_conv_time += intervalVirtTime;
    runData.virtualConvTime_perIter.push_back(intervalVirtTime);
		float* tmp = _nextState;
		_nextState = _state;
		_state = tmp;
		this->computeOutput(_state);

		if (!change) {
			runData.virtualConvTime = virtual_conv_time;


			runData.totalConvTime = unit_conv_times[0];
			runData.cnnP_convTime.push_back(unit_conv_times[0]);
			for (unsigned int i = 1; i < _num_CNN_P; i++) {
				runData.cnnP_convTime.push_back(unit_conv_times[i]);
				if (unit_conv_times[i] > runData.totalConvTime) {
					runData.totalConvTime = unit_conv_times[i];
				}
			}

			for (unsigned int i = 0; i < stream.numPartitions(); i++) {
				runData.partition_convTime.push_back(partition_conv_times[i]);
			}
			return runData;
		}
	}
	std::cout << "ERROR: Reached iteration limit" << std::endl;
	return runData;
}

SP_CNN::RunData SP_CNN::earlyFinishIntervalRunFastProp(CNNGene* gene, CNNInput* input, unsigned int interval_N, std::string partition_order)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);
	assert(_num_CNN_P == 1);

	unsigned int unit_conv_times[_num_CNN_P];
	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
		unit_conv_times[i] = 0;
	}


	PartitionStream stream = get_partition_stream(partition_order, _M, _N, _cnnM, _cnnN);
	unsigned int partition_conv_times[stream.numPartitions()];
	for (unsigned int i = 0; i < stream.numPartitions(); i++) {
		partition_conv_times[i] = 0;
	}



	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	SP_CNN::RunData runData;

	unsigned int virtual_conv_time = 0;
	for (unsigned int i = 1; i <= SP_CNN_RUN_ITERATION_LIMIT; i++) {
		bool change = false;
		unsigned int intervalVirtTime = 0;
		for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
			Partition p = stream.getPartition(j);


			int curCNN_P = 0;
			int curCNNval = unit_conv_times[curCNN_P];
			for (unsigned int k = 1; k < _num_CNN_P; k++) {
				if (unit_conv_times[k] < curCNNval) {
					curCNN_P = k;
					curCNNval = unit_conv_times[k];
				}
			}
			CNN_P* cnnUnit = _CNN_P_units[curCNN_P];
			initialize_CNN_P(p, cnnUnit, u);
			unsigned int convTime = cnnUnit->convergenceRun(interval_N);
			float* cnnState = cnnUnit->state();
			size_t indR = p.indR1() + _r;
			size_t indC = p.indC1() + _r;
			for (size_t i = 0; i < _cnnM; ++i) {
				memcpy(&_state[(indR + i) * _C + indC], &cnnState[(i + _r) * (_cnnN + 2 * _r) + _r], sizeof(float) * _cnnN);
			}

			change |= (convTime > 1) ? true : false;

			if (convTime > intervalVirtTime) {
				intervalVirtTime = convTime;
			}
			unit_conv_times[curCNN_P] += convTime;
			partition_conv_times[j] += convTime;
		}


		int curCNNval = 0;
		// One partition finished processing, so set current
		// completion to time of last executing partition.
		for (unsigned int k = 0; k < _num_CNN_P; k++) {
			if (unit_conv_times[k] > curCNNval) {
				curCNNval = unit_conv_times[k];
			}
		}

		for (unsigned int k = 0; k < _num_CNN_P; k++) {
			if (unit_conv_times[k] > curCNNval) {
				unit_conv_times[k] = curCNNval;
			}
		}

		virtual_conv_time += intervalVirtTime;


		this->computeOutput(_state);

		if (!change) {
			runData.virtualConvTime = virtual_conv_time;


			runData.totalConvTime = unit_conv_times[0];
			runData.cnnP_convTime.push_back(unit_conv_times[0]);
			for (unsigned int i = 1; i < _num_CNN_P; i++) {
				runData.cnnP_convTime.push_back(unit_conv_times[i]);
				if (unit_conv_times[i] > runData.totalConvTime) {
					runData.totalConvTime = unit_conv_times[i];
				}
			}

			for (unsigned int i = 0; i < stream.numPartitions(); i++) {
				runData.partition_convTime.push_back(partition_conv_times[i]);
			}
			return runData;
		}
	}
	std::cout << "ERROR: Reached iteration limit" << std::endl;
	return runData;
}

SP_CNN::RunData SP_CNN::earlyFinishIntervalRunConvOpt(CNNGene* gene, CNNInput* input, unsigned int interval_N, std::string partition_order)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);


	unsigned int unit_conv_times[_num_CNN_P];
	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
		unit_conv_times[i] = 0;
	}


	PartitionStream stream = get_partition_stream(partition_order, _M, _N, _cnnM, _cnnN);
	unsigned int partition_conv_times[stream.numPartitions()];
	for (unsigned int i = 0; i < stream.numPartitions(); i++) {
		partition_conv_times[i] = 0;
	}

	bool partitionChanged[stream.dimX()+2][stream.dimY()+2];
	bool partitionConverged[stream.dimX()][stream.dimY()];

	bool oldPartitionChanged[stream.dimX()+2][stream.dimY()+2];
	bool oldPartitionConverged[stream.dimX()][stream.dimY()];

	for (unsigned int i = 0; i < (stream.dimX() + 2); i++) {
		for (unsigned int j = 0; j < (stream.dimY() + 2); j++) {
			if ((i == 0) || (i == (stream.dimX() + 1)) ||
					(j == 0) || (j == (stream.dimY() + 1))) {
				partitionChanged[i][j] = false;
			} else {
				partitionChanged[i][j] = true;
			}
		}
	}

	for (unsigned int i = 0; i < stream.dimX(); i++) {
		for (unsigned int j = 0; j < stream.dimY(); j++) {
			partitionConverged[i][j] = false;
			oldPartitionConverged[i][j] = false;
		}
	}

	for (unsigned int i = 0; i < stream.dimX() + 2; i++) {
		for (unsigned int j = 0; j < stream.dimY() + 2; j++) {
			oldPartitionChanged[i][j] = partitionChanged[i][j];
		}
	}

	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	SP_CNN::RunData runData;

	unsigned int virtual_conv_time = 0;
	for (unsigned int i = 1; i <= SP_CNN_RUN_ITERATION_LIMIT; i++) {
		bool change = false;
		unsigned int intervalVirtTime = 0;
		for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
			Partition p = stream.getPartition(j);

			bool canSkip = true;
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if ((i == 0) && (j == 0)) {
						canSkip &= oldPartitionConverged[p.idX()][p.idY()];
						//canSkip &= ~oldPartitionChanged[p.idX() + 1][p.idY() + 1];
					} else {
						canSkip &= ~oldPartitionChanged[p.idX() + 1 + i][p.idY() + 1 + j];
					}
				}
			}

			if (canSkip)
				continue;


			int curCNN_P = 0;
			int curCNNval = unit_conv_times[curCNN_P];
			for (unsigned int k = 1; k < _num_CNN_P; k++) {
				if (unit_conv_times[k] < curCNNval) {
					curCNN_P = k;
					curCNNval = unit_conv_times[k];
				}
			}
			CNN_P* cnnUnit = _CNN_P_units[curCNN_P];


			initialize_CNN_P(p, cnnUnit, u);
			unsigned int convTime = cnnUnit->convergenceRun(interval_N);
			copy_CNN_P_state(p, cnnUnit);

			if (convTime > 1) {
				partitionChanged[p.idX() + 1][p.idY() + 1] = true;
			} else {
				partitionChanged[p.idX() + 1][p.idY() + 1] = false;
			}

			if (convTime < interval_N) {
				partitionConverged[p.idX()][p.idY()] = true;
			} else {
				partitionConverged[p.idX()][p.idY()] = false;
			}

			change |= (convTime > 1) ? true : false;

			if (convTime > intervalVirtTime) {
				intervalVirtTime = convTime;
			}
			unit_conv_times[curCNN_P] += convTime;
			partition_conv_times[j] += convTime;
		}


		int curCNNval = 0;
		// One partition finished processing, so set current
		// completion to time of last executing partition.
		for (unsigned int k = 0; k < _num_CNN_P; k++) {
			if (unit_conv_times[k] > curCNNval) {
				curCNNval = unit_conv_times[k];
			}
		}

		for (unsigned int k = 0; k < _num_CNN_P; k++) {
			if (unit_conv_times[k] > curCNNval) {
				unit_conv_times[k] = curCNNval;
			}
		}

		virtual_conv_time += intervalVirtTime;

		for (unsigned int i = 0; i < stream.dimX() + 2; i++) {
			for (unsigned int j = 0; j < stream.dimY() + 2; j++) {
				oldPartitionChanged[i][j] = partitionChanged[i][j];
			}
		}

		for (unsigned int i = 0; i < stream.dimX(); i++) {
			for (unsigned int j = 0; j < stream.dimY(); j++) {
				oldPartitionConverged[i][j] = partitionConverged[i][j];
			}
		}

		float* tmp = _nextState;
		_nextState = _state;
		_state = tmp;
		this->computeOutput(_state);

		if (!change) {
			runData.virtualConvTime = virtual_conv_time;


			runData.totalConvTime = unit_conv_times[0];
			runData.cnnP_convTime.push_back(unit_conv_times[0]);
			for (unsigned int i = 1; i < _num_CNN_P; i++) {
				runData.cnnP_convTime.push_back(unit_conv_times[i]);
				if (unit_conv_times[i] > runData.totalConvTime) {
					runData.totalConvTime = unit_conv_times[i];
				}
			}

			for (unsigned int i = 0; i < stream.numPartitions(); i++) {
				runData.partition_convTime.push_back(partition_conv_times[i]);
			}
			return runData;
		}
	}
	std::cout << "ERROR: Reached iteration limit" << std::endl;
	return runData;
}

SP_CNN::RunData SP_CNN::naiveRunNoShareBoundary(CNNGene* gene, float* image)
{
	assert(gene->r() == _r);

	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
	}


	PartitionStream stream = get_partition_stream("row-major", _M, _N, _cnnM, _cnnN);

	float* tmpImage = new float[_cnnM * _cnnN];

	// Copy the initial state over
	int curCNN_P = 0;
	SP_CNN::RunData runData;
	// Run each partition till it converges
	for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
		Partition p = stream.getPartition(j);
		size_t indR = p.indR1();
		size_t indC = p.indC1();

		// Copy partition to tmpImage
		for (size_t i = 0; i < _cnnM; ++i) {
			memcpy(&tmpImage[i * _cnnN], &image[(indR + i) * _N + indC], sizeof(float) * _cnnN);
		}

		CNNInput* input = gene->getInput(tmpImage, _cnnM, _cnnN);

		CNN_P* cnnUnit = _CNN_P_units[curCNN_P];
		float* cnnState = cnnUnit->state();
		float* cnnU = cnnUnit->u();
		float* state = input->initialState();
		float* u = input->u();


		memcpy(cnnState, state, sizeof(float) * (_cnnN + 2* _r) * (_cnnM + 2 * _r));
		memcpy(cnnU, u, sizeof(float) * (_cnnN + 2* _r) * (_cnnM + 2 * _r));


		unsigned int tmp_conv_time = cnnUnit->convergenceRun(CNN_CONVERGENCE_LIMIT);

		for (size_t i = 0; i < _cnnM; ++i) {
			memcpy(&_nextState[(indR + i + _r) * _C + indC + _r], &cnnState[(i + _r) * (_cnnN + 2 * _r) + _r], sizeof(float) * _cnnN);
		}
		delete input;

		if (tmp_conv_time > runData.virtualConvTime)
			runData.virtualConvTime = tmp_conv_time;
		runData.totalConvTime += tmp_conv_time;
	}
	computeOutput(_nextState);
	return runData;
}

SP_CNN::RunData SP_CNN::naiveRun(CNNGene* gene, CNNInput* input)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);

	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
	}


	PartitionStream stream = get_partition_stream("row-major", _M, _N, _cnnM, _cnnN);

	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	int curCNN_P = 0;
	SP_CNN::RunData runData;
	// Run each partition till it converges
	for (unsigned int j = 0; j < stream.numPartitions(); ++j) {
		Partition p = stream.getPartition(j);


		CNN_P* cnnUnit = _CNN_P_units[curCNN_P];
		initialize_CNN_P(p, cnnUnit, u);
		unsigned int tmp_conv_time = cnnUnit->convergenceRun(CNN_CONVERGENCE_LIMIT);
		copy_CNN_P_state(p, cnnUnit);
		if (tmp_conv_time > runData.virtualConvTime)
			runData.virtualConvTime = tmp_conv_time;
		runData.totalConvTime += tmp_conv_time;
	}
	computeOutput(_nextState);
	return runData;
}

void SP_CNN::computeOutput(float* state)
{
	for (size_t i = 0; i < _M; ++i) {
		for (size_t j = 0; j < _N; ++j) {
			float stateVal = state[(i + _r) * _C + (j + _r)];
			_output[(i + _r) * _C + (j + _r)] = (abs(stateVal + 1) - abs(stateVal - 1)) / 2.0f;
		}
	}
}

void SP_CNN::copyOutput(float* out)
{
	for (size_t i = 0; i < _M; ++i) {
		for (size_t j = 0; j < _N; ++j) {
			out[i* _N + j] = _output[(i + _r) * _C + (j + _r)];
		}
	}
}

void SP_CNN::initialize_CNN_P(Partition p, CNN_P* cnnUnit, const float* u)
{
	float* cnnState = cnnUnit->state();
	float* cnnU = cnnUnit->u();
	size_t indR = p.indR1();
	size_t indC = p.indC1();
	for (size_t i = 0; i < (_cnnM + 2 * _r); ++i) {
		memcpy(&cnnState[i * (_cnnN + 2 * _r)], &_state[(indR + i) * _C + indC], sizeof(float) * (_cnnN + 2 * _r));
		memcpy(&cnnU[i * (_cnnN + 2 * _r)], &u[(indR + i) * _C + indC], sizeof(float) * (_cnnN + 2 * _r));
	}
}

void SP_CNN::copy_CNN_P_state(Partition p, CNN_P* cnnUnit)
{
	float* cnnState = cnnUnit->state();
	size_t indR = p.indR1() + _r;
	size_t indC = p.indC1() + _r;
	for (size_t i = 0; i < _cnnM; ++i) {
		memcpy(&_nextState[(indR + i) * _C + indC], &cnnState[(i + _r) * (_cnnN + 2 * _r) + _r], sizeof(float) * _cnnN);
	}
}

//////////////////////////////////////////////////////////////
// CNN_P
//////////////////////////////////////////////////////////////
CNN_P::CNN_P(size_t M, size_t N, size_t r)
{
	_M = M;
	_N = N;
	_r = r;

	_R = M + 2 * r;
	_C = N + 2 * r;
	_gene = NULL;


	_state = new float[_R * _C];
	_output = new float[_R * _C];
	_u = new float[_R * _C];
}

CNN_P::~CNN_P()
{
	delete[] _state;
	delete[] _output;
	delete[] _u;
}

bool CNN_P::run(unsigned int N)
{
	assert(_gene != NULL);
	bool change = false;
	computeInitialOutput();
	for (unsigned int i = 1; i <= N; ++i) {
		change |= computeNextState();
		computeOutput();
	}
	return change;
}

unsigned int CNN_P::convergenceRun(unsigned int N)
{
	assert(_gene != NULL);
	bool change = true;
	computeInitialOutput();
	int i = 0;
	while ((i < N) && change) {
		change = computeNextState();
		computeOutput();
		i = i + 1;
	}
	return i;
}


unsigned int CNN_P::convergenceRun()
{
	assert(_gene != NULL);
	bool change = true;
	computeInitialOutput();
	int i = 0;
	while (change) {
		change = computeNextState();
		computeOutput();
		i = i + 1;
	}
	return i;
}


bool CNN_P::computeNextState()
{
	const float z = _gene->z();
	const float* A  = _gene->A();
	const float* B = _gene->B();

	bool change = false;
	for (size_t i = 0; i < _M; ++i) {
		for (size_t j = 0; j < _N; ++j) {
			float delta = z;
			for (size_t k = 0; k < (2 * _r + 1); k++) {
				for (size_t l = 0; l < (2 * _r + 1); l++) {
					delta += A[k * (2 * _r + 1) + l] * _output[(i + k) * _C + (j + l)];
					delta += B[k * (2 * _r + 1) + l] * _u[(i + k) * _C + (j + l)];
				}
			}

			if (abs(_state[(i + _r) * _C + (j + _r)] - delta) >= CHANGE_THRESHOLD) {
				change = true;
			}

			_state[(i + _r) * _C + (j + _r)] = delta;
		}
	}
	return change;
}

void CNN_P::computeInitialOutput()
{
	for (size_t i = 0; i < _R; ++i) {
		for (size_t j = 0; j < _C; ++j) {
			float stateVal = _state[i * _C + j];
			_output[i * _C + j] = (abs(stateVal + 1) - abs(stateVal - 1)) / 2.0f;
		}
	}
}

void CNN_P::computeOutput()
{
	for (size_t i = 0; i < _M; ++i) {
		for (size_t j = 0; j < _N; ++j) {
			float stateVal = _state[(i + _r) * _C + (j + _r)];
			_output[(i + _r) * _C + (j + _r)] = (abs(stateVal + 1) - abs(stateVal - 1)) / 2.0f;
		}
	}
}
