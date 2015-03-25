/*
 * SP_CNN_CPU.h
 *
 *  Created on: Apr 10, 2014
 *      Author: manatunga
 */

#ifndef SP_CNN_H_
#define SP_CNN_H_

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <algorithm>
#include <vector>
#include "PartitionStream.h"
#include "cnn/cnn.h"
#include "DRAMSim2-2.2.2/DRAMSim.h"
#include <assert.h>
#include <iostream>

using namespace DRAMSim;

#define DEBUG 0
#define KEYPRESS 0


class PartitionExecutionStats {
public:
	Partition p;

	uint32_t partitionId;
	uint32_t assignedCore;
	uint64_t assignedCycle;
	uint32_t virt_conv_time;
	uint64_t execTime;
	int64_t execTimeDiff;
	uint64_t waitTime;
	uint64_t coreTime;
	uint64_t readTime;
	uint64_t writeTime;

	uint64_t priority_partition_cycle;
	uint64_t execute_ready_cycle;
	uint64_t execute_start_cycle;
	uint64_t execute_done_cycle;
	uint64_t finished_cycle;

public:
	PartitionExecutionStats(uint32_t pId)
	{
		partitionId = pId;
		assignedCore = 0;
		assignedCycle = 0;
		virt_conv_time = 0;
		execTime = 0;
		execTimeDiff = 0;
		readTime = 0;
		writeTime = 0;
		waitTime = 0;
		coreTime = 0;

		execute_ready_cycle = 0;
		execute_start_cycle = 0;
		execute_done_cycle = 0;
		finished_cycle = 0;
		priority_partition_cycle = 0;
	}

	void computeTimes() {
    assert(execute_start_cycle >= priority_partition_cycle);
		waitTime = execute_start_cycle - priority_partition_cycle;
		coreTime = finished_cycle - assignedCycle;
		readTime = execute_start_cycle - assignedCycle;
		writeTime = finished_cycle - execute_done_cycle - 1; // -1 cycle for time to indicate scheduler
		int64_t execTime2 =  ((int64_t)execute_done_cycle - (int64_t)execute_start_cycle);
    execTimeDiff = execTime - execTime2;
	}
};

class PartitionExecution {
public:
	Partition p;
	PartitionExecutionStats* stats;

	enum ExecutionState {
		FETCHING_DATA=0,
		WAITING_TO_EXECUTE=1,
		EXECUTING=2,
		WRITING_DATA=3,
		INFORM_SCHEDULER_DONE=4,
		FINISHED=5
	};

	ExecutionState state;

private:
	uint64_t _uBaseAddr;
	uint64_t _stateBaseAddr;
	uint64_t _nextStateBaseAddr;
	uint32_t bytes_per_mem_request;
	uint32_t bytes_per_cnn_element;

	bool _allReadRequestsIssued;
	bool _allWriteRequestsIssued;

	bool curReadForInput;
	size_t curReadIndR, curReadIndC;
	size_t curWriteIndR, curWriteIndC;

	size_t indR1, indR2, indC1, indC2;


	uint64_t numReadRequestsIssued;
	uint64_t numWriteRequestsIssued;

	uint64_t numReadRequestsCompleted;
	uint64_t numWriteRequestsCompleted;

	size_t _dataC;

	uint64_t numCyclesExecuted;
	uint64_t numExecutionCycles;

public:
	PartitionExecution(Partition partition, PartitionExecutionStats* s, size_t M, size_t N, size_t r,
			uint64_t uBaseAddr, uint64_t stateBaseAddr, uint64_t nextStateBaseAddr, size_t C,
			uint32_t bytes_per_mem, uint32_t bytes_per_cnn);

	uint64_t getNextReadAddr();
	uint64_t getNextWriteAddr();

	bool allReadRequestsIssued() { return _allReadRequestsIssued; }
	bool allWriteRequsesIssued() { return _allWriteRequestsIssued; }

	void read_complete(uint64_t cycle) {
		numReadRequestsCompleted++;
#if DEBUG
			std::cout << numReadRequestsCompleted << " reads completed" << std::endl;
			std::cout << numReadRequestsIssued << " reads remaining" << std::endl;
#endif

		if (_allReadRequestsIssued && (numReadRequestsCompleted == numReadRequestsIssued)) {

			state = WAITING_TO_EXECUTE;
			stats->execute_ready_cycle = cycle;
		}
	}

	bool execute_a_cycle(uint64_t cycle) {
		if (++numCyclesExecuted >= numExecutionCycles) {
			stats->execute_done_cycle = cycle;
			state = WRITING_DATA;
      return true;
		}
    return false;
	}

	void write_complete(uint64_t cycle) {
		numWriteRequestsCompleted++;
#if DEBUG
			std::cout << numWriteRequestsCompleted << " writes completed" << std::endl;
			std::cout << numWriteRequestsIssued << " writes remaining" << std::endl;
#endif
		if (_allWriteRequestsIssued && (numWriteRequestsCompleted == numWriteRequestsIssued)) {
			state = INFORM_SCHEDULER_DONE;
		}
	}

	void setNumExecutionCycles(uint64_t num) { numExecutionCycles = num; }


#if DEBUG
public:
	std::string stateAsString()
	{
		switch (state) {
		case FETCHING_DATA:
			return "FETCHING_DATA";
		case WAITING_TO_EXECUTE:
			return "WAITING_TO_EXECUTE";
		case EXECUTING:
			return "EXECUTING";
		case WRITING_DATA:
			return "WRITING_DATA";
		case INFORM_SCHEDULER_DONE:
			return "INFORM_SCHEDULER_DONE";
		case FINISHED:
			return "FINISHED";
		default:
			return "UNKNOWN";
		}
	}
#endif
};

class SP_CNN;
class CNN_P {
public:
	class RunData {
	public:
		uint32_t convTime;
		bool change;
		uint64_t numExecutionCycles;

	public:
		RunData()
		{
			convTime = 0;
			change = false;
			numExecutionCycles = 0;
		}

		RunData(uint32_t conv, bool c, uint64_t n)
		{
			convTime = conv;
			change = c;
			numExecutionCycles = n;
		}
	};

private:
	class MSHRentry {
	public:
		uint64_t _addr;
		PartitionExecution* _execPartition;
		bool _isWrite;

		MSHRentry(uint64_t addr, PartitionExecution* execPartId, bool isWrite) {
			_addr = addr;
			_execPartition = execPartId;
			_isWrite = isWrite;
		}

		uint64_t addr() { return _addr; }
		PartitionExecution* execPartition() { return _execPartition; }
		bool isWrite() { return _isWrite; }
	};

private:
	uint32_t _id;

	size_t _M;
	size_t _N;
	size_t _r;

	CNNGene* _gene;
	float* _state;
	float* _output;
	float* _u;

	size_t _R;
	size_t _C;



private:
	// Timing simulator additions
	SP_CNN* _spCNN;
	MultiChannelMemorySystem* _memSystem;
	std::vector<MSHRentry> mshr;
	std::vector<PartitionExecution*> executingPartitions;
	uint32_t max_num_exec_partitions;


	uint64_t _curCycleCount;
	uint64_t _numCyclesToRun;

	uint32_t num_mshr_entries;
	uint32_t max_num_mem_requests;
	uint32_t bytes_per_mem_request;
	uint32_t bytes_per_cnn_element;
	uint32_t num_alus_per_pe;
	uint32_t cell_to_pe_ratio;

	uint64_t compute_constants_cost;
	uint64_t compute_state_cost;
	uint64_t compute_output_cost;



public:
	CNN_P(SP_CNN* sp_cnn, uint32_t id, size_t M, size_t N, size_t r, MultiChannelMemorySystem* memSystem,
			uint32_t num_mshr, uint32_t max_num_mem_req, uint32_t bytes_per_mem, uint32_t bytes_per_elem,
			uint32_t num_alus, uint32_t cell_to_pe, uint32_t max_num_assigned_partitions);
	~CNN_P();
	void run_a_cycle();
	CNN_P::RunData run(unsigned int N, std::string cnn_run_type);

	CNN_P::RunData fixedIntervalRun(unsigned int N);
	CNN_P::RunData convergenceRun(unsigned int N);
	CNN_P::RunData convergenceRun();


	float* state() { return _state; }
	float* u() { return _u; }

	void setGene(CNNGene* gene) { _gene = gene; }

	void assignPartition(Partition tmpP, PartitionExecutionStats* stats,
			uint64_t uBaseAddr, uint64_t stateBaseAddr, uint64_t nextStateBaseAddr, size_t C);

	bool read_complete(uint64_t addr);
	bool write_complete(uint64_t addr);

	bool executeUnitFree() {
		if (executingPartitions.empty())
			return true;

		for (std::vector<PartitionExecution*>::iterator it = executingPartitions.begin();
				it != executingPartitions.end(); it++) {
			PartitionExecution* exec = *it;
			if (exec->state <= PartitionExecution::ExecutionState::EXECUTING)
				return false;
		}
		return true;
	}

	bool canAssignPartition() {
		return executingPartitions.size() < max_num_exec_partitions;
	}

	bool noPartitionsRemaining() {
		return executingPartitions.empty();
	}
#if DEBUG
	void printStats();
#endif

private:
	void memController_run_a_cycle();
	bool computeNextState();
	void computeInitialOutput();
	void computeOutput();

};


#define CHANGE_THRESHOLD 0.00001
#define COMPARE_THRESHOLD 0.00001

#define SP_CNN_RUN_ITERATION_LIMIT 500
#define CNN_CONVERGENCE_LIMIT 5000


class SP_CNN {
public:
	class RunData {
	public:
		uint32_t numIters;
		uint32_t numPartitions;
		uint32_t virtualConvTime;
		uint64_t cycle_count;
		double actualTimeMs;

		std::vector<std::vector<PartitionExecutionStats*>> execution_stats;

		bool error;
		std::string error_msg;

	public:
		RunData() {
			numIters = 0;
			virtualConvTime = 0;
			actualTimeMs = 0.0;
			cycle_count = 0;
			error = false;
			error_msg = "";
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

private:
	uint64_t _cycleCount;
	uint64_t _cnnClkFreqHz;
	uint64_t _uBaseAddr;
	uint64_t _stateBaseAddr;
	uint64_t _nextStateBaseAddr;
	MultiChannelMemorySystem *_memSystem;
	unsigned _memSize;

public:
	SP_CNN(size_t M, size_t N, size_t r, size_t cnnM, size_t cnnN, uint8_t num_CNN_P,
			std::string memInstanceFile, std::string systemIniFile, std::string rel_dir,
			unsigned mem_size, uint32_t num_mshr, uint32_t max_num_mem_req, uint32_t bytes_per_mem_request, uint32_t bytes_per_cnn_elem,
			uint64_t cnnClkFreqHz, uint32_t num_alus, uint32_t cell_to_pe, uint32_t max_num_assigned_partitions);
	~SP_CNN();

	RunData run(CNNGene* gene, CNNInput* input,
			unsigned int interval_N, std::string partition_order, std::string type);

	void copyOutput(float* out);
	void read_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
	void write_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
	uint64_t cur_cycle_num() { return _cycleCount; }

private:
	void computeOutput(float* state);
	void initialize_CNN_P(Partition p, CNN_P* cnnUnit, const float* state, const float* u);
	void copy_CNN_P_state(Partition p, CNN_P* cnnUnit, float* state);

};




#endif /* SP_CNN_H_ */
