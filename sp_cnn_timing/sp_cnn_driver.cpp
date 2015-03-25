#include <string>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include "utilities/OptionPrinter.hpp"
#include "utilities/BWImage.h"
#include "cnn/cnn.h"
#include "SP_CNN.h"
#include "boost/program_options.hpp"

using namespace std;
namespace po = boost::program_options;


int main(int argc, char** argv)
{
	// SP_CNN defaults
	unsigned int numCNNunits = 1;
	std::string memInstanceFile = "ini/DDR3_micron_16M_8B_x8_sg15.ini";
	std::string systemIniFile = "system.ini";
	std::string rel_dir = ".";
	unsigned mem_size = 2048;
	uint32_t num_mshr = 64;
	uint32_t max_num_out_mem_req = 1;
	uint32_t bytes_per_mem_request = 64;
	uint32_t bytes_per_cnn_elem = 1;
	uint64_t cnnClkFreqHz = 200 * 1000 * 1000;
	uint32_t num_alus = 1;
	uint32_t cell_to_pe_ratio = 40;
	uint32_t max_num_assigned_partitions = 1;

	std::string geneName;
	std::string inputFile;
	unsigned int cnnM, cnnN;
	unsigned int interval;
	std::string part_trav_order;
	std::string run_type;
	std::string correctOutFile;

	po::options_description desc("Allowed options");

	std::string program(argv[0]);

	desc.add_options()
					("help,h", "produce help message")
					// Required positional parameters
					("gene_name", po::value<std::string>(&geneName)->required(), "cnn gene")
					("input_file", po::value<std::string>(&inputFile)->required(), "input file")
					("run_type", po::value<std::string>(&run_type)->required(), "cnn_p run-type either (fixed-interval or early-finish)")
					("cnnM", po::value<unsigned int>(&cnnM)->required(), "cnn_p number of rows")
					("cnnN", po::value<unsigned int>(&cnnN)->required(), "cnn_p number of columns")
					("interval_length", po::value<unsigned int>(&interval)->required(), "interval length")
					("part_trav_order", po::value<std::string>(&part_trav_order)->required(), "partition traversal order")
					("correct_output", po::value<std::string>(&correctOutFile)->required(), "correct output file")
					// Optional parameters
					("num_cnnP", po::value<unsigned int>(&numCNNunits), "number of CNN_P units")
					("dram_mem_file", po::value<std::string>(&memInstanceFile), "DRAM SIM 2 memory specification file")
					("dram_sys_file", po::value<std::string>(&systemIniFile), "DRAM SIM 2 system specification file")
					("dram_rel_dir", po::value<std::string>(&rel_dir), "DRAM SIM 2 relative directory of filenames")
					("mem_size", po::value<unsigned>(&mem_size), "memory size (in MB)")
					("cnn_freq", po::value<uint64_t>(&cnnClkFreqHz), "CNN_P frequency (in MHz)")
					("mshr_size", po::value<uint32_t>(&num_mshr), "number of mshr entries for CNN_P")
					("max_num_mem_req", po::value<uint32_t>(&max_num_out_mem_req), "max number of mem requests per cycle for CNN_P")
					("bytes_per_mem_req", po::value<uint32_t>(&bytes_per_mem_request), "bytes per memory request")
					("bytes_per_cnn_elem", po::value<uint32_t>(&bytes_per_cnn_elem), "bytes per single cnn element value")
					("num_alus", po::value<uint32_t>(&num_alus), "number of alus used by CNN_P (must be 1 or 2 currently)")
					("cell_to_pe_ratio", po::value<uint32_t>(&cell_to_pe_ratio), "the cell to pe ratio for CNN_P")
					("max_num_assigned_part", po::value<uint32_t>(&max_num_assigned_partitions), "maximum number of assigned partitions for CNN_P")
					;

	po::positional_options_description positionalOptions;
	positionalOptions.add("gene_name", 1);
	positionalOptions.add("input_file", 1);
	positionalOptions.add("run_type", 1);
	positionalOptions.add("cnnM", 1);
	positionalOptions.add("cnnN", 1);
	positionalOptions.add("interval_length", 1);
	positionalOptions.add("part_trav_order", 1);
	positionalOptions.add("correct_output", 1);


	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc)
			.positional(positionalOptions).run(), vm);

	if (vm.count("help")) {
		rad::OptionPrinter::printStandardAppDesc(program, std::cout, desc, &positionalOptions);
		return 0;
	}

	po::notify(vm);

	if (vm.count("cnn_freq"))
		cnnClkFreqHz *= 1000 * 1000;
  

	CNNGene* gene = CNNGeneFactory::createCNNGene(geneName);


	BWImage testIn, correctOutput;
	testIn.dlmread(inputFile, " ");
	correctOutput.dlmread(correctOutFile, " ");
	CNNInput* input = gene->getInput(testIn.getImage(), testIn.R(), testIn.C());


	SP_CNN sp_cnn(testIn.R(), testIn.C(),  gene->r(), cnnM, cnnN, numCNNunits,
			memInstanceFile, systemIniFile, rel_dir,
			mem_size, num_mshr, max_num_out_mem_req, bytes_per_mem_request, bytes_per_cnn_elem,
			cnnClkFreqHz, num_alus, cell_to_pe_ratio, max_num_assigned_partitions);

	SP_CNN::RunData runData = sp_cnn.run(gene, input, interval, part_trav_order, run_type);

	BWImage sp_cnnOut(testIn.R(), testIn.C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << geneName << "(" << inputFile << "," << numCNNunits << "," << interval << ")";
	cout << ": (" << runData.virtualConvTime << "," << runData.cycle_count << "," << runData.actualTimeMs;
	unsigned int numPixelsDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
	double percentDiff = ((double)numPixelsDiff) / ((double)(testIn.R() * testIn.C()));
	cout << "," << numPixelsDiff << "," << percentDiff << ")" << endl;

	cout << std::endl;
	cout << std::endl;

	ofstream file;
	file.open("sim_config.cfg");
	file << "GENE: " << geneName << std::endl;
	file << "INPUT_FILE: " << inputFile << std::endl;
	file << "RUN_TYPE: " << run_type << std::endl;
	file << "CNN_M: " << cnnM << std::endl;
	file << "CNN_N: " << cnnN << std::endl;
	file << "INTERVAL: " << interval << std::endl;
	file << "PART_TRAV_ORDER: " << part_trav_order << std::endl;
	file << "CORRECT_OUTPUT: " << correctOutFile << std::endl;
	file << "NUM_CNNP: " << numCNNunits << std::endl;
	file << "DRAM_MEM_FILE: " << memInstanceFile << std::endl;
	file << "DRAM_SYS_FILE: " << systemIniFile << std::endl;
	file << "DRAM_REL_DIR: " << rel_dir << std::endl;
	file << "MEM_SIZE: " << mem_size << std::endl;
	file << "CNN_FREQ: " << cnnClkFreqHz << std::endl;
	file << "MSHR_SIZE: " << num_mshr << std::endl;
	file << "MAX_NUM_MEM_REQ: " << max_num_out_mem_req << std::endl;
	file << "BYTES_PER_MEM_REQ: " << bytes_per_mem_request << std::endl;
	file << "BYTES_PER_CNN_ELEM: " << bytes_per_cnn_elem << std::endl;
	file << "NUM_ALUS: " << num_alus << std::endl;
	file << "CELL_TO_PE_RATIO: " << cell_to_pe_ratio << std::endl;
	file << "MAX_NUM_ASSIGNED_PART: " << max_num_assigned_partitions << std::endl;
	file.close();
	
	for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
			it != runData.execution_stats.end(); it++) {
		for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
			PartitionExecutionStats* partExec = *jt;;
			partExec->computeTimes();
		}
	}

	for (int i = 0; i < runData.numPartitions; i++) {
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			int count = 0;
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					count++;
				}
			}
			assert(count == 1);
		}
	}


	file.open("virt_conv_time.csv");
	file << "Virtual Convergence Time";
	for (int i = 1; i <= runData.numIters; i++) {
		file << "," << i;
	}
	file << std::endl;

	for (int i = 0; i < runData.numPartitions; i++) {
		file << i;
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					file << "," << partExec->virt_conv_time;
				}
			}
		}
		file << std::endl;
	}
	file.close();

	file.open("core_time.csv");
	file << "Core Time";
	for (int i = 1; i <= runData.numIters; i++) {
		file << "," << i;
	}
	file << std::endl;

	for (int i = 0; i < runData.numPartitions; i++) {
		file << i;
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					file << "," << partExec->coreTime;
				}
			}
		}
		file << std::endl;
	}
	file.close();


	file.open("exec_time.csv");
	file << "Exec Time";
	for (int i = 1; i <= runData.numIters; i++) {
		file << "," << i;
	}
	file << std::endl;

	for (int i = 0; i < runData.numPartitions; i++) {
		file << i;
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					file << "," << partExec->execTime;
				}
			}
		}
		file << std::endl;
	}
	file.close();


	file.open("exec_time_diff.csv");
	file << "Exec Time Diff";
	for (int i = 1; i <= runData.numIters; i++) {
		file << "," << i;
	}
	file << std::endl;

	for (int i = 0; i < runData.numPartitions; i++) {
		file << i;
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					file << "," << partExec->execTimeDiff;
				}
			}
		}
		file << std::endl;
	}
	file.close();

	file.open("read_time.csv");
	file << "Read Time";
	for (int i = 1; i <= runData.numIters; i++) {
		file << "," << i;
	}
	file << std::endl;

	for (int i = 0; i < runData.numPartitions; i++) {
		file << i;
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					file << "," << partExec->readTime;
				}
			}
		}
		file << std::endl;
	}
	file.close();


	file.open("write_time.csv");
	file << "Write Time";
	for (int i = 1; i <= runData.numIters; i++) {
		file << "," << i;
	}
	file << std::endl;

	for (int i = 0; i < runData.numPartitions; i++) {
		file << i;
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					file << "," << partExec->writeTime;
				}
			}
		}
		file << std::endl;
	}
	file.close();


	file.open("wait_time.csv");
	file << "Wait Time";
	for (int i = 1; i <= runData.numIters; i++) {
		file << "," << i;
	}
	file << std::endl;

	for (int i = 0; i < runData.numPartitions; i++) {
		file << i;
		for (std::vector<std::vector<PartitionExecutionStats*>>::iterator it = runData.execution_stats.begin();
				it != runData.execution_stats.end(); it++) {
			for (std::vector<PartitionExecutionStats*>::iterator jt = it->begin(); jt != it->end(); jt++) {
				PartitionExecutionStats* partExec = *jt;;
				if (partExec->partitionId == i) {
					file << "," << partExec->waitTime;
				}
			}
		}
		file << std::endl;
	}
	file.close();

	return 0;
}
