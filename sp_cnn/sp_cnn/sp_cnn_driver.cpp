#include <string>
#include <iostream>
#include "utilities/BWImage.h"
#include "cnn/cnn.h"
#include "SP_CNN.h"
using namespace std;


int main(int argc, char* argv[])
{
	int num_args_needed = 11;
	if (argc != num_args_needed)
	{
		// Error on necessary parameters
		cerr << "Invalid number of arguments. Expected " << num_args_needed << " arguments. Received " << argc << " arguments." << endl;
		return -1;
	}

	string run_type = string(argv[1]);
	unsigned int cnnM = atoi(argv[2]);
	unsigned int cnnN = atoi(argv[3]);
	unsigned int numCNNunits = atoi(argv[4]);
	unsigned int interval = atoi(argv[5]);
	string geneName = string(argv[6]);
	string testInputFile = string(argv[7]);
	string order = string(argv[8]);
	string correctOutFile = string(argv[9]);
	string run_param = string(argv[10]);


	CNNGene* gene = CNNGeneFactory::createCNNGene(geneName);

	BWImage testIn, correctOutput;
	testIn.dlmread(testInputFile, " ");
	correctOutput.dlmread(correctOutFile, " ");
	CNNInput* input = gene->getInput(testIn.getImage(), testIn.R(), testIn.C());

	SP_CNN sp_cnn(testIn.R(), testIn.C(),  gene->r(), cnnM, cnnN, numCNNunits);
	SP_CNN::RunData runData;
	if (run_type == "fixed-interval") {
		if (run_param == "slow")
			runData = sp_cnn.fixedIntervalRun(gene, input, interval, order);
		else if (run_param == "fast")
			runData = sp_cnn.fixedIntervalRunFastProp(gene, input, interval, order);
		else if (run_param == "conv")
			runData = sp_cnn.fixedIntervalRunConvOpt(gene, input, interval, order);
		else {
			cerr << "Invalid Run-Parameter" << endl;
			return 0;
		}
	} else if (run_type == "early-finish") {
		if (run_param == "slow")
			runData = sp_cnn.earlyFinishIntervalRun(gene, input, interval, order);
		else if (run_param == "fast")
			runData = sp_cnn.earlyFinishIntervalRunFastProp(gene, input, interval, order);
		else if (run_param == "conv")
			runData = sp_cnn.earlyFinishIntervalRunConvOpt(gene, input, interval, order);
		else {
			cerr << "Invalid Run-Parameter" << endl;
			return 0;
		}
	} else {
		cout << "ERROR" << endl;
		delete gene;
		delete input;
		return 0;
	}

	BWImage sp_cnnOut(testIn.R(), testIn.C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << geneName << "(" << testInputFile << "," << numCNNunits << "," << interval << ")";
	cout << ": (" << runData.totalConvTime << "," << runData.virtualConvTime;
	unsigned int numPixelsDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
	double percentDiff = ((double)numPixelsDiff) / ((double)(testIn.R() * testIn.C()));
	cout << "," << numPixelsDiff << "," << percentDiff << ")" << endl;

	if (run_type == "early-finish") {
		cout << "CNN_P conv. Times - " << geneName << "(" << testInputFile << "," << numCNNunits << "," << interval << ")";
		cout << ": (";
		cout << runData.cnnP_convTime[0];
		for (unsigned int i = 1; i < runData.cnnP_convTime.size(); i++) {
			cout << "," << runData.cnnP_convTime[i];
		}
		cout << ")" << endl;
		cout << "Partition conv. Times - " << geneName << "(" << testInputFile << "," << numCNNunits << "," << interval << ")";
		cout << ": (";
		cout << runData.partition_convTime[0];
		for (unsigned int i = 1; i < runData.partition_convTime.size(); i++) {
			cout << "," << runData.partition_convTime[i];
		}
		cout << ")" << endl;
		cout << "Virtual conv. Times - " << geneName << "(" << testInputFile << "," << numCNNunits << "," << interval << ")";
		cout << ": (";
    unsigned int totalTime = runData.virtualConvTime_perIter[0];
		cout << runData.virtualConvTime_perIter[0] << ",";
		for (unsigned int i = 1; i < runData.virtualConvTime_perIter.size(); i++) {
			cout << "," << runData.virtualConvTime_perIter[i];
      totalTime += runData.virtualConvTime_perIter[i];
		}
		cout << totalTime << ")" << endl;
	}

  if (numPixelsDiff > 0)
  	sp_cnnOut.dlmwrite("incorrect_" + testInputFile, " ");

	delete gene;
	delete input;
	return 0;
}
