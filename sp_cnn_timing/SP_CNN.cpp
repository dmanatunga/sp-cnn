#include "SP_CNN.h"
#include <cmath>
#include <string.h>
#include <assert.h>
#include <iostream>

#include "PartitionGenerator.h"
#include "cnn/cnn.h"

/* FIXME: this may be broken, currently */
void power_callback(double a, double b, double c, double d)
{
	//	printf("power callback: %0.3f, %0.3f, %0.3f, %0.3f\n",a,b,c,d);
}


//////////////////////////////////////////////////////////////
// SP_CNNmgeaint
//////////////////////////////////////////////////////////////
SP_CNN::SP_CNN(size_t M, size_t N, size_t r, size_t cnnM, size_t cnnN, uint8_t num_CNN_P,
		std::string memInstanceFile, std::string systemIniFile, std::string rel_dir,
		unsigned mem_size, uint32_t num_mshr, uint32_t max_num_mem_req, uint32_t bytes_per_mem_request, uint32_t bytes_per_cnn_elem,
		uint64_t cnnClkFreqHz, uint32_t num_alus, uint32_t cell_to_pe, uint32_t max_num_assigned_partitions)
{
	_M = M;
	_N = N;
	_r = r;

	_R = M + 2 * r;
	_C = N + 2 * r;


	_num_CNN_P = num_CNN_P;
	_cnnM = cnnM;
	_cnnN = cnnN;

	_state = new float[_R * _C];
	_nextState = new float[_R * _C];
	_output = new float[_R * _C];

	// Timing related parameters
	_cycleCount = 0;
	_cnnClkFreqHz = cnnClkFreqHz;
	_memSize = mem_size;
	//_memSystem = getMemorySystemInstance("ini/DDR2_micron_16M_8b_x8_sg3E.ini", "system.ini", ".", "sp_cnn", mem_size, NULL);
	_memSystem = getMemorySystemInstance(memInstanceFile, systemIniFile, rel_dir, "sp_cnn", mem_size, NULL);
	_memSystem->setCPUClockSpeed(_cnnClkFreqHz);

	TransactionCompleteCB *read_cb = new Callback<SP_CNN, void, unsigned, uint64_t, uint64_t>(this, &SP_CNN::read_complete);
	TransactionCompleteCB *write_cb = new Callback<SP_CNN, void, unsigned, uint64_t, uint64_t>(this, &SP_CNN::write_complete);
	_memSystem->RegisterCallbacks(read_cb, write_cb, power_callback);

	_uBaseAddr = 0;
	uint64_t arraySize = _R * _C * bytes_per_cnn_elem;
	_stateBaseAddr =  (_uBaseAddr + arraySize + bytes_per_mem_request - 1) & ~(bytes_per_mem_request -1);

	assert((_uBaseAddr + arraySize)  < _stateBaseAddr);
	_nextStateBaseAddr = (_stateBaseAddr + arraySize + bytes_per_mem_request - 1) & ~(bytes_per_mem_request -1);

	assert((_stateBaseAddr + arraySize) < _nextStateBaseAddr);
	assert((_nextStateBaseAddr + arraySize) < (mem_size * 1024LL * 1024LL));
	assert((_uBaseAddr & (bytes_per_mem_request - 1)) == 0);
	assert((_stateBaseAddr & (bytes_per_mem_request - 1)) == 0);
	assert((_nextStateBaseAddr & (bytes_per_mem_request - 1)) == 0);

	for (uint8_t i = 0; i < _num_CNN_P; i++) {
		CNN_P* cnn_p = new CNN_P(this, i, _cnnM, _cnnN, _r, _memSystem,
							     num_mshr, max_num_mem_req, bytes_per_mem_request, bytes_per_cnn_elem,
				                 num_alus, cell_to_pe, max_num_assigned_partitions);
		_CNN_P_units.push_back(cnn_p);
	}


#if DEBUG
	std::cout << "SP-CNN: (M=" << _M << ", N=" << _N << ", cnnM=" << _cnnM << ", cnnN=" << _cnnN << ", numCNN=" << _num_CNN_P;
	std::cout << ", CPU_CLK=" << (_cnnClkFreqHz / (1000 * 1000)) << " MHz, ";
  std::cout << "Prefetch=" << max_num_assigned_partitions<< std::endl;
	std::cout << "Memory: (uBaseAddr=" << _uBaseAddr << ", stateBaseAddr=" << _stateBaseAddr << ", nextStateBaseAddr=" << _nextStateBaseAddr << ")" << std::endl;
#endif
}

SP_CNN::~SP_CNN()
{
	for (int i = 0; i < _num_CNN_P; i++) {
		delete _CNN_P_units[i];
	}
	delete[] _state;
	delete[] _nextState;
	delete[] _output;
	delete _memSystem;
}

SP_CNN::RunData SP_CNN::run(CNNGene* gene, CNNInput* input, unsigned int interval_N, std::string partition_order, std::string cnn_run_type)
{
	assert(gene->r() == _r);
	assert(input->R() == _R);
	assert(input->C() == _C);

	// Copy the initial state over
	memcpy(_state, input->initialState(), sizeof(float) * _R * _C);
	const float* u = input->u();

	SP_CNN::RunData runData;
	_cycleCount = 0;
	uint32_t numIterations = 0;
	uint32_t max_virt_conv_for_iteration = 0;

	for (int i = 0; i < _num_CNN_P; i++) {
		_CNN_P_units[i]->setGene(gene);
	}

	PartitionStream stream = get_partition_stream(partition_order, _M, _N, _cnnM, _cnnN);
	uint32_t streamPos = 0;
	bool change = false;
	std::vector<PartitionExecutionStats*> iterStats;

	while (true) {
#if DEBUG
		std::cout << "======================================================" << std::endl;
		std::cout << "Cycle " << _cycleCount << std::endl;
#endif
		// Loop through CNN_P units and run for a cycle
		for (int i = 0; i < _num_CNN_P; i++) {
			_CNN_P_units[i]->run_a_cycle();
#if DEBUG
			_CNN_P_units[i]->printStats();
#endif
		}
#if DEBUG && KEYPRESS
		std::cin.get();
#endif

		// In same cycle, we also assign a partition if one is available.
		// This occurs after so that we don't assign a partition and then begin issuing memory
		// requests in the same cycle
		if (streamPos < stream.numPartitions()) {
			bool assignedPartition = false;

			for (int i = 0; i < _num_CNN_P; i++) {
				CNN_P* cnnUnit = _CNN_P_units[i];

				if (cnnUnit->canAssignPartition() && cnnUnit->executeUnitFree()) {
#if DEBUG
					std::cout << "-- Assigning Partition " << streamPos << "-> CNN_P " << i << std::endl;
#endif
					Partition p = stream.getPartition(streamPos);



					// Copy over the cnn state
					initialize_CNN_P(p, cnnUnit, _state, u);

					// Run the cnn and have it determine the number of cycles it will execute
					CNN_P::RunData cnnRunData = cnnUnit->run(interval_N, cnn_run_type);

					change |= cnnRunData.change;
					max_virt_conv_for_iteration = (max_virt_conv_for_iteration < cnnRunData.convTime) ? cnnRunData.convTime : max_virt_conv_for_iteration;
					// Copy over the computed state
					copy_CNN_P_state(p, cnnUnit, _nextState);

					// Initialize some basic stats data
					PartitionExecutionStats* stats = new PartitionExecutionStats(streamPos);
					stats->assignedCore = i;
					stats->assignedCycle = this->cur_cycle_num();
					stats->virt_conv_time = cnnRunData.convTime;
					stats->execTime = cnnRunData.numExecutionCycles;
					iterStats.push_back(stats);
					cnnUnit->assignPartition(p, stats, _uBaseAddr, _stateBaseAddr, _nextStateBaseAddr, _C);

					// Assigned one partition, so cannot assign anymore
					assignedPartition = true;
					streamPos++;
					break;
				}
			}

			if (!assignedPartition) {
				for (int i = 0; i < _num_CNN_P; i++) {
					CNN_P* cnnUnit = _CNN_P_units[i];
					if (cnnUnit->canAssignPartition()) {
#if DEBUG
						std::cout << "-- Assigning Prefetch Partition " << streamPos << "-> CNN_P " << i << std::endl;
#endif
						Partition p = stream.getPartition(streamPos);

						// Copy over the cnn state
						initialize_CNN_P(p, cnnUnit, _state, u);

						// Run the cnn and have it determine the number of cycles it will execute
						CNN_P::RunData cnnRunData = cnnUnit->run(interval_N, cnn_run_type);

						change |= cnnRunData.change;
						max_virt_conv_for_iteration = (max_virt_conv_for_iteration < cnnRunData.convTime) ? cnnRunData.convTime : max_virt_conv_for_iteration;
						// Copy over the computed state
						copy_CNN_P_state(p, cnnUnit, _nextState);

						// Initialize some basic stats data
						PartitionExecutionStats* stats = new PartitionExecutionStats(streamPos);
						stats->assignedCore = i;
						stats->assignedCycle = this->cur_cycle_num();
						stats->virt_conv_time = cnnRunData.convTime;
						stats->execTime = cnnRunData.numExecutionCycles;
						iterStats.push_back(stats);

						cnnUnit->assignPartition(p, stats, _uBaseAddr, _stateBaseAddr, _nextStateBaseAddr, _C);

						// Assigned one partition, so cannot assign anymore
						assignedPartition = true;
						streamPos++;
						break;
					}
				}
			}
		}

		// Check to see if any CNN unit is a non-waiting state
		bool noActiveAssignmentsRemaining = true;
		for (int i = 0; i < _num_CNN_P; i++) {
			if (!_CNN_P_units[i]->noPartitionsRemaining()) {
				noActiveAssignmentsRemaining = false;
				break;
			}
		}

		// Update memory system by one clock cycle
		_memSystem->update();
		_cycleCount++;

		// If there are no active cnn units, we should be done with current iteration.
		// We need to check to see if no changes were made, in which case we are done, else, we
		// need to repeat process
		if (noActiveAssignmentsRemaining && (streamPos >= stream.numPartitions())) {

			// Swap states
			float* tmp = _nextState;
			_nextState = _state;
			_state = tmp;

			// Swap Base addresses
			uint64_t tmpAddr = _nextStateBaseAddr;
			_nextStateBaseAddr = _stateBaseAddr;
			_stateBaseAddr = tmpAddr;

			numIterations++;
			runData.virtualConvTime += max_virt_conv_for_iteration;
			runData.execution_stats.push_back(iterStats);
			iterStats.clear();
#if DEBUG
			std::cout << "-- Finished Iteration " << numIterations << std::endl;
#endif
			if (change) {
				if (numIterations >= SP_CNN_RUN_ITERATION_LIMIT) {
					runData.error = true;
					runData.error_msg = "Hit iteration limit";
					return runData;
				}
				// Reset to start of partition stream
				change = false;
				streamPos = 0;
			} else {
				// Program finished
#if DEBUG
				std::cout << "-- Program Complete" << std::endl;
#endif
				break;
			}
		}
	}

	this->computeOutput(_state);
	runData.numIters = numIterations;
	runData.numPartitions = stream.numPartitions();
	runData.cycle_count = _cycleCount;
	runData.actualTimeMs = (((double)_cycleCount) / ((double) _cnnClkFreqHz)) * 1000;
	return runData;
}


void SP_CNN::read_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
	// TODO: Different CNN-Ps could get back read request for same address
	// In this case, we need some mechanism where we don't issue two reads to same location
	// This could be solved using a global mshr to only issue unique reads.
	for (int i = 0; i < _num_CNN_P; i++) {
		if (_CNN_P_units[i]->read_complete(address))
			return;
	}
	assert(false);
}

void SP_CNN::write_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
	// TODO: Different CNN-Ps could get back write request for same address
	// In this case, we need some mechanism where we don't issue two reads to same location
	// This could be solved using a global mshr to only issue unique reads.
	for (int i = 0; i < _num_CNN_P; i++) {
		if (_CNN_P_units[i]->write_complete(address))
			return;
	}
	assert(false);
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

void SP_CNN::initialize_CNN_P(Partition p, CNN_P* cnnUnit, const float* state, const float* u)
{
	float* cnnState = cnnUnit->state();
	float* cnnU = cnnUnit->u();
	size_t indR = p.indR1();
	size_t indC = p.indC1();
	for (size_t i = 0; i < (_cnnM + 2 * _r); ++i) {
		memcpy(&cnnState[i * (_cnnN + 2 * _r)], &state[(indR + i) * _C + indC], sizeof(float) * (_cnnN + 2 * _r));
		memcpy(&cnnU[i * (_cnnN + 2 * _r)], &u[(indR + i) * _C + indC], sizeof(float) * (_cnnN + 2 * _r));
	}
}

void SP_CNN::copy_CNN_P_state(Partition p, CNN_P* cnnUnit, float* state)
{
	float* cnnState = cnnUnit->state();
	size_t indR = p.indR1() + _r;
	size_t indC = p.indC1() + _r;
	for (size_t i = 0; i < _cnnM; ++i) {
		memcpy(&state[(indR + i) * _C + indC], &cnnState[(i + _r) * (_cnnN + 2 * _r) + _r], sizeof(float) * _cnnN);
	}
}


//////////////////////////////////////////////////////////////
// CNN_P
//////////////////////////////////////////////////////////////
CNN_P::CNN_P(SP_CNN* sp_cnn, uint32_t id, size_t M, size_t N, size_t r, MultiChannelMemorySystem* memSystem,
		uint32_t num_mshr, uint32_t max_num_mem_req, uint32_t bytes_per_mem, uint32_t bytes_per_elem,
		uint32_t num_alus, uint32_t cell_to_pe, uint32_t max_num_assigned_partitions)
{
	_spCNN = sp_cnn;
	_id = id;
	_M = M;
	_N = N;
	_r = r;

	_R = M + 2 * r;
	_C = N + 2 * r;
	_gene = NULL;


	_state = new float[_R * _C];
	_output = new float[_R * _C];
	_u = new float[_R * _C];

	_curCycleCount = -1;
	_numCyclesToRun = -1;

	cell_to_pe_ratio = cell_to_pe;

	num_mshr_entries = num_mshr;
	max_num_mem_requests = max_num_mem_req;
	bytes_per_mem_request = bytes_per_mem;
	bytes_per_cnn_element = bytes_per_elem;

	assert(max_num_assigned_partitions != 0);
	max_num_exec_partitions = max_num_assigned_partitions;

	num_alus_per_pe = num_alus;

	if (num_alus_per_pe == 1) {
		compute_output_cost = 3 * cell_to_pe_ratio;
		compute_constants_cost = 10 * cell_to_pe_ratio;
		compute_state_cost = 10 * cell_to_pe_ratio;
	} else if (num_alus == 2) {
		compute_constants_cost = 6 * cell_to_pe_ratio;
		compute_state_cost = 6 * cell_to_pe_ratio;
		compute_output_cost = 2 * cell_to_pe_ratio;
	} else {
		assert(false);
	}

	_memSystem = memSystem;
}

CNN_P::~CNN_P()
{
	delete[] _state;
	delete[] _output;
	delete[] _u;
}

void CNN_P::assignPartition(Partition tmpP, PartitionExecutionStats* stats, uint64_t uBaseAddr, uint64_t stateBaseAddr, uint64_t nextStateBaseAddr, size_t C)
{
	assert(executingPartitions.size() < max_num_exec_partitions);
	if (executingPartitions.empty()) {
		stats->priority_partition_cycle = _spCNN->cur_cycle_num();
	}
	executingPartitions.push_back(new PartitionExecution(tmpP, stats, _M, _N, _r,
														 uBaseAddr, stateBaseAddr, nextStateBaseAddr, C,
														 bytes_per_mem_request, bytes_per_cnn_element));
}

void CNN_P::run_a_cycle() {

	if (!executingPartitions.empty()){
		// Remove head partition if finished
		std::vector<PartitionExecution*>::iterator it = executingPartitions.begin();
		PartitionExecution* partExec = *it;
		if (partExec->state == PartitionExecution::ExecutionState::FINISHED) {
#if DEBUG
			std::cout << "Removing executing partition " << partExec->stats->partitionId;
			std::cout << " from core " << _id << "." << std::endl;
#endif
			it = executingPartitions.erase(it);
		}
	}

	// Must be done first, since following actions can cause changes to states
	memController_run_a_cycle(); // Running memory controller for a cycle

	uint8_t numExecPartitions = 0;

	bool foundExecPart = false;
  bool done = false;
	for (std::vector<PartitionExecution*>::iterator it = executingPartitions.begin();
			it != executingPartitions.end(); it++) {

		PartitionExecution* partExec = *it;
		if (!foundExecPart) {
			if (partExec->state <= PartitionExecution::ExecutionState::EXECUTING)
				foundExecPart = true;

			switch (partExec->state) {
			case PartitionExecution::ExecutionState::WAITING_TO_EXECUTE:
				// Transition from waiting to execute to execute since this is
				// the first partition found that in less
				partExec->stats->execute_start_cycle = _spCNN->cur_cycle_num();
				partExec->state = PartitionExecution::ExecutionState::EXECUTING;
				// TODO: This is probably a really bad place to put this
				partExec->setNumExecutionCycles(partExec->stats->execTime);
#if DEBUG
				std::cout << "Core " << _id << " starting execution of partition " << partExec->stats->partitionId;
				std::cout << " for " << partExec->stats->execTime << " cycles";
				std::cout << "." << std::endl;
#endif

#if DEBUG && KEYPRESS
				std::cin.get();
#endif
			case PartitionExecution::ExecutionState::EXECUTING:
				done = partExec->execute_a_cycle(_spCNN->cur_cycle_num());
        it++;
        if (it != executingPartitions.end()) {
          partExec = *it;
          partExec->stats->priority_partition_cycle = _spCNN->cur_cycle_num() + 1;
        }
        it--;
				break;
			case PartitionExecution::ExecutionState::INFORM_SCHEDULER_DONE:
				partExec->stats->finished_cycle = _spCNN->cur_cycle_num();
				partExec->state = PartitionExecution::ExecutionState::FINISHED;
        
#if DEBUG && KEYPRESS
				std::cin.get();
#endif
				break;
			}
		} else {
			// All other  partitions should not be executing, if we found a partition
			// that is already executing
			assert(partExec->state < PartitionExecution::ExecutionState::EXECUTING);
		}
	}
}

void CNN_P::memController_run_a_cycle() {
	uint8_t numRequestsIssued = 0;

	if (mshr.size() == num_mshr_entries) {
#if DEBUG
		std::cout << "CNN-P(" << _id << ")" << " MSHR full." << std::endl;
#endif
		return;
	}

	// Reads get priority over writes
	for (std::vector<PartitionExecution*>::iterator it = executingPartitions.begin();
			it != executingPartitions.end(); it++) {

		PartitionExecution* partExec = *it;
		if (partExec->state == PartitionExecution::ExecutionState::FETCHING_DATA)
		{
			while (!partExec->allReadRequestsIssued()) {
				uint64_t addr = partExec->getNextReadAddr();
				mshr.push_back(MSHRentry(addr, partExec, false));
				_memSystem->addTransaction(false, addr);
#if DEBUG
				std::cout << "CNN-P (" << _id << ") issuing read for address " << addr;
				std::cout << " from partition " << partExec->stats->partitionId << "." << std::endl;
#endif
				if (++numRequestsIssued >= max_num_mem_requests)
					return;
			}
		}


	}


	for (std::vector<PartitionExecution*>::iterator it = executingPartitions.begin();
			it != executingPartitions.end(); it++) {

		PartitionExecution* partExec = *it;
		if (partExec->state == PartitionExecution::ExecutionState::WRITING_DATA)
		{
			while (!partExec->allWriteRequsesIssued()) {
				uint64_t addr = partExec->getNextWriteAddr();
				mshr.push_back(MSHRentry(addr, partExec, true));
				_memSystem->addTransaction(true, addr);
#if DEBUG
				std::cout << "CNN-P (" << _id << ") issuing write for address " << addr;
				std::cout << " from partition " << partExec->stats->partitionId << "." << std::endl;
#endif
				if (++numRequestsIssued >= max_num_mem_requests)
					return;
			}
		}
	}
}

CNN_P::RunData CNN_P::run(unsigned int N, std::string cnn_run_type)
{
	CNN_P::RunData runData;
	if (cnn_run_type == "fixed-interval") {
		return fixedIntervalRun(N);
	} else if (cnn_run_type == "early-finish") {
		return convergenceRun(N);
	} else {
		std::cerr << "Invalid run type" << cnn_run_type << std::endl;
		exit(0);
	}
}

CNN_P::RunData CNN_P::fixedIntervalRun(unsigned int N)
{
	assert(_gene != NULL);

	_numCyclesToRun = compute_constants_cost;
	bool change = false;
	computeInitialOutput();
	for (unsigned int i = 1; i <= N; ++i) {
		change |= computeNextState();
		computeOutput();
	}

	return RunData(N, change, _numCyclesToRun);
}

CNN_P::RunData CNN_P::convergenceRun(unsigned int N)
{
	assert(_gene != NULL);

	_numCyclesToRun = compute_constants_cost;
	bool change = true;
	computeInitialOutput();
	int i = 0;
	while ((i < N) && change) {
		change = computeNextState();
		computeOutput();
		i = i + 1;
	}
	return RunData(i, i > 1, _numCyclesToRun);
}


CNN_P::RunData CNN_P::convergenceRun()
{
	assert(_gene != NULL);

	_numCyclesToRun = compute_constants_cost;
	bool change = true;
	computeInitialOutput();
	int i = 0;
	while (change) {
		change = computeNextState();
		computeOutput();
		i = i + 1;
	}
	return RunData(i, i > 1, _numCyclesToRun);
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

	_numCyclesToRun += compute_state_cost;
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

	_numCyclesToRun += compute_output_cost;
}

void CNN_P::computeOutput()
{
	for (size_t i = 0; i < _M; ++i) {
		for (size_t j = 0; j < _N; ++j) {
			float stateVal = _state[(i + _r) * _C + (j + _r)];
			_output[(i + _r) * _C + (j + _r)] = (abs(stateVal + 1) - abs(stateVal - 1)) / 2.0f;
		}
	}

	_numCyclesToRun += compute_output_cost;
}



bool CNN_P::read_complete(uint64_t addr)
{
	for (std::vector<MSHRentry>::iterator it = mshr.begin(); it != mshr.end(); ++it) {
		if (it->addr() == addr) {
#if DEBUG
			std::cout << "CNN-P (" << _id << ") received read for address " << addr;
			std::cout << " for partition "<< it->execPartition()->stats->partitionId << "." << std::endl;
#endif
			assert(!it->isWrite());
			it->execPartition()->read_complete(_spCNN->cur_cycle_num() + 1);
			mshr.erase(it);
			return true;
		}
	}
	return false;
}

bool CNN_P::write_complete(uint64_t addr)
{
	for (std::vector<MSHRentry>::iterator it = mshr.begin(); it != mshr.end(); ++it) {
		if (it->addr() == addr) {
#if DEBUG
			std::cout << "CNN-P (" << _id << ") received write for address " << addr;
			std::cout << " for partition "<< it->execPartition()->stats->partitionId << "." << std::endl;
#endif
			assert(it->isWrite());
			it->execPartition()->write_complete(_spCNN->cur_cycle_num() + 1);
			mshr.erase(it);
			return true;
		}
	}
	return false;
}

#if DEBUG
void CNN_P::printStats()
{
	unsigned int i = 0;
	if (executingPartitions.empty()) {
		std::cout << "EMPTY" << std::endl;
		return;
	}
	for (std::vector<PartitionExecution*>::iterator it = executingPartitions.begin();
			it != executingPartitions.end(); it++) {
		PartitionExecution* exec = *it;
		std::cout << "P(" << exec->stats->partitionId << ", " << exec->stateAsString() << ") ";
	}
	std::cout << std::endl;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// PartitionExecution
//////////////////////////////////////////////////////////////////////////////
PartitionExecution::PartitionExecution(Partition partition, PartitionExecutionStats* s, size_t M, size_t N, size_t r,
		uint64_t uBaseAddr, uint64_t stateBaseAddr, uint64_t nextStateBaseAddr, size_t C,
		uint32_t bytes_per_mem, uint32_t bytes_per_cnn)
{
	p = partition;
	stats = s;

	numReadRequestsIssued = 0;
	numReadRequestsCompleted = 0;
	numWriteRequestsIssued = 0;
	numWriteRequestsCompleted = 0;

	_allReadRequestsIssued = false;
	_allWriteRequestsIssued = false;

	curWriteIndR = indR1 = p.indR1();
	curWriteIndC = indC1 = p.indC1();

	indR2 = indR1 + M + 2 * r;
	indC2 = indC1 + N + 2 * r;

	curReadIndR = p.indR1();
	curReadIndC = p.indC1();

	curReadForInput = true;
	_dataC = C;

	state = FETCHING_DATA;

	_uBaseAddr = uBaseAddr;
	_stateBaseAddr = stateBaseAddr;
	_nextStateBaseAddr = nextStateBaseAddr;

	bytes_per_mem_request = bytes_per_mem;
	bytes_per_cnn_element = bytes_per_cnn;

	numExecutionCycles = 0;
	numCyclesExecuted = 0;
}

uint64_t PartitionExecution::getNextReadAddr()
{
	assert(!_allReadRequestsIssued);

	uint64_t offset = (curReadIndR * _dataC + curReadIndC) * bytes_per_cnn_element;
	uint64_t addr;

	if (curReadForInput)
		addr = _uBaseAddr + offset;
	else
		addr = _stateBaseAddr + offset;
	addr &= ~(bytes_per_mem_request-1);

	numReadRequestsIssued += 1;

	uint64_t remainder = offset & (bytes_per_mem_request - 1);
	if (remainder) {
		curReadIndC += (bytes_per_mem_request - remainder) / bytes_per_cnn_element;
	} else {
		curReadIndC += bytes_per_mem_request / bytes_per_cnn_element;
	}


	if (curReadIndC >= indC2) {
		curReadIndC = indC1;
		if (++curReadIndR >= indR2) {
			if (curReadForInput) {
				curReadIndR = indR1;
				curReadIndC = indC1;
				curReadForInput = false;
			} else {
				_allReadRequestsIssued = true;
			}
		}
	}
	return addr;
}

uint64_t PartitionExecution::getNextWriteAddr()
{
	assert(!_allWriteRequestsIssued);

	uint64_t offset = (curWriteIndR * _dataC + curWriteIndC) * bytes_per_cnn_element;
	uint64_t addr = _nextStateBaseAddr + (offset & ~(bytes_per_mem_request-1));
	addr &= ~(bytes_per_mem_request-1);

	numWriteRequestsIssued += 1;

	uint64_t remainder = offset & (bytes_per_mem_request - 1);
	if (remainder) {
		curWriteIndC += (bytes_per_mem_request - remainder) / bytes_per_cnn_element;
	} else {
		curWriteIndC += bytes_per_mem_request / bytes_per_cnn_element;
	}

	if (curWriteIndC >= indC2) {
		curWriteIndC = indC1;
		if (++curWriteIndR >= indR2) {
			_allWriteRequestsIssued = true;
		}
	}
	return addr;
}




