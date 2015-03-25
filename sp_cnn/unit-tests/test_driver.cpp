#include <string>
#include <iostream>
#include "utilities/BWImage.h"
#include "cnn/cnn.h"
#include "SP_CNN.h"
using namespace std;


void testSP_CNN_naive(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, ",");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, 1);
	SP_CNN::RunData runData = sp_cnn.naiveRun(gene, input);
	BWImage naiveOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(naiveOut.getImage());

	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;

	bool pass1 = correctOutput.compare(&naiveOut);

	if (pass1) {
		cout << "Naive Test Passed!" << endl;
	} else {
		cout << "Naive Test Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&naiveOut);
		cout << "Naive Test: " << numDiff << endl;
	}
}

void testSP_CNN_naive_no_share(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, " ");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, 1);
	SP_CNN::RunData runData = sp_cnn.naiveRunNoShareBoundary(gene, testIn->getImage());

	BWImage naiveOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(naiveOut.getImage());

	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;

	bool pass1 = correctOutput.compare(&naiveOut);

	if (pass1) {
		cout << "Naive (No-Share Boundary) Test Passed!" << endl;
	} else {
		cout << "Naive (No-Share Boundary Test Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&naiveOut);
		cout << "Naive (No-Share Boundary Test: " << numDiff << endl;
	}
}

void testSP_CNN_fixed(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, unsigned int interval, string order, unsigned int numUnits, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, " ");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, numUnits);
	SP_CNN::RunData runData = sp_cnn.fixedIntervalRun(gene, input, interval, order);
	BWImage sp_cnnOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << "Number Iterations: " << runData.numIters << endl;
	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;

	bool pass1 = correctOutput.compare(&sp_cnnOut);

	if (pass1) {
		cout << "Fixed Interval Passed!" << endl;
	} else {
		cout << "Fixed Interval Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
		cout << "Fixed Interval Test: " << numDiff << endl;
	}
}

void testSP_CNN_fixed_fast_prop(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, unsigned int interval, string order, unsigned int numUnits, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, " ");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, numUnits);
	SP_CNN::RunData runData = sp_cnn.fixedIntervalRunFastProp(gene, input, interval, order);
	BWImage sp_cnnOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << "Number Iterations: " << runData.numIters << endl;
	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;

	bool pass1 = correctOutput.compare(&sp_cnnOut);

	if (pass1) {
		cout << "Fixed Interval (Fast-Prop) Passed!" << endl;
	} else {
		cout << "Fixed Interval (Fast-Prop) Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
		cout << "Fixed Interval (Fast-Prop) Test: " << numDiff << endl;
	}
}

void testSP_CNN_fixed_conv_opt(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, unsigned int interval, string order, unsigned int numUnits, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, " ");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, numUnits);
	SP_CNN::RunData runData = sp_cnn.fixedIntervalRunConvOpt(gene, input, interval, order);
	BWImage sp_cnnOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << "Number Iterations: " << runData.numIters << endl;
	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;

	bool pass1 = correctOutput.compare(&sp_cnnOut);

	if (pass1) {
		cout << "Fixed Interval (Conv-Opt) Passed!" << endl;
	} else {
		cout << "Fixed Interval (Conv-Opt) Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
		cout << "Fixed Interval (Conv-Opt) Test: " << numDiff << endl;
	}
}

void testSP_CNN_early_finish(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, unsigned int interval, string order, unsigned int numUnits, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, " ");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, numUnits);
	SP_CNN::RunData runData = sp_cnn.earlyFinishIntervalRun(gene, input, interval, order);
	BWImage sp_cnnOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;
	cout << "CNN-P conv. Times: (";
	cout << runData.cnnP_convTime[0];
	for (unsigned int i = 1; i < runData.cnnP_convTime.size(); i++) {
		cout << "," << runData.cnnP_convTime[i];
	}
	cout << ")" << endl;
	cout << "Partition conv. Times: (";
	cout << runData.partition_convTime[0];
	for (unsigned int i = 1; i < runData.partition_convTime.size(); i++) {
		cout << "," << runData.partition_convTime[i];
	}
	cout << ")" << endl;

	bool pass1 = correctOutput.compare(&sp_cnnOut);

	if (pass1) {
		cout << "Early Finish Passed!" << endl;
	} else {
		cout << "Early Finish Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
		cout << "Early Finish Interval Test: " << numDiff << endl;
	}
}

void testSP_CNN_early_finish_fast_prop(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, unsigned int interval, string order, unsigned int numUnits, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, " ");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, numUnits);
	SP_CNN::RunData runData = sp_cnn.earlyFinishIntervalRunFastProp(gene, input, interval, order);
	BWImage sp_cnnOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;
	cout << "CNN-P conv. Times: (";
	cout << runData.cnnP_convTime[0];
	for (unsigned int i = 1; i < runData.cnnP_convTime.size(); i++) {
		cout << "," << runData.cnnP_convTime[i];
	}
	cout << ")" << endl;
	cout << "Partition conv. Times: (";
	cout << runData.partition_convTime[0];
	for (unsigned int i = 1; i < runData.partition_convTime.size(); i++) {
		cout << "," << runData.partition_convTime[i];
	}
	cout << ")" << endl;

	bool pass1 = correctOutput.compare(&sp_cnnOut);

	if (pass1) {
		cout << "Early Finish (Fast-Prop) Passed!" << endl;
	} else {
		cout << "Early Finish (Fast-Prop) Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
		cout << "Early Finish (Fast-Prop) Interval Test: " << numDiff << endl;
	}
}


void testSP_CNN_early_finish_conv_opt(CNNGene* gene, CNNInput* input, size_t cnnM, size_t cnnN, BWImage* testIn, unsigned int interval, string order, unsigned int numUnits, string correctOutFile)
{
	BWImage correctOutput;
	correctOutput.dlmread(correctOutFile, " ");
	SP_CNN sp_cnn(testIn->R(), testIn->C(),  gene->r(), cnnM, cnnN, numUnits);
	SP_CNN::RunData runData = sp_cnn.earlyFinishIntervalRunConvOpt(gene, input, interval, order);
	BWImage sp_cnnOut(testIn->R(), testIn->C());
	sp_cnn.copyOutput(sp_cnnOut.getImage());

	cout << "Total Convergence Time: " << runData.totalConvTime << endl;
	cout << "Virtual Convergence Time: " << runData.virtualConvTime << endl;
	cout << "CNN-P conv. Times: (";
	cout << runData.cnnP_convTime[0];
	for (unsigned int i = 1; i < runData.cnnP_convTime.size(); i++) {
		cout << "," << runData.cnnP_convTime[i];
	}
	cout << ")" << endl;
	cout << "Partition conv. Times: (";
	cout << runData.partition_convTime[0];
	for (unsigned int i = 1; i < runData.partition_convTime.size(); i++) {
		cout << "," << runData.partition_convTime[i];
	}
	cout << ")" << endl;

	bool pass1 = correctOutput.compare(&sp_cnnOut);

	if (pass1) {
		cout << "Early Finish (Conv-Opt) Passed!" << endl;
	} else {
		cout << "Early Finish (Conv-Opt) Failed..." << endl;
		unsigned int numDiff = correctOutput.numPixelsDiff(&sp_cnnOut);
		cout << "Early Finish (Conv-Opt) Interval Test: " << numDiff << endl;
		sp_cnnOut.dlmwrite("incorrect.dlm", " ");
	}
}

int main(int argc, char* argv[])
{
	int num_args_needed;

	if (argc <= 1)
	{
		// Error on necessary parameters
		cerr << "Invalid number of arguments." << endl;
		return -1;
	}

	string type = string(argv[1]);
	if (type == "naive") {
		num_args_needed = 7;
	} else if (type == "naive-no-share") {
		num_args_needed = 7;
	} else if (type == "fixed-interval") {
		num_args_needed = 10;
	} else if (type == "early-finish") {
		num_args_needed = 10;
	} else if (type == "fixed-interval-fast") {
		num_args_needed = 10;
	} else if (type == "early-finish-fast") {
		num_args_needed = 10;
	} else if (type == "fixed-interval-conv") {
		num_args_needed = 10;
	} else if (type == "early-finish-conv") {
		num_args_needed = 10;
	} else {
		cerr << "Invalid input type argument." << endl;
		return -1;
	}

	if (argc != num_args_needed)
	{
		// Error on necessary parameters
		cerr << "Invalid number of arguments. Expected " << num_args_needed-1 << " arguments. Received " << argc << " arguments." << endl;
		return -1;
	}

	unsigned int cnnM = atoi(argv[2]);
	unsigned int cnnN = atoi(argv[3]);
	string geneName = string(argv[4]);
	string testInputFile = string(argv[5]);

	CNNGene* gene = CNNGeneFactory::createCNNGene(geneName);

	BWImage testIn;
	testIn.dlmread(testInputFile, " ");
	CNNInput* input = gene->getInput(testIn.getImage(), testIn.R(), testIn.C());


	if (type == "naive") {
		cout << "NaiveTest: " << testInputFile << endl;
		string naiveCorrectFile = string(argv[6]);
		testSP_CNN_naive(gene, input, cnnM, cnnN, &testIn, naiveCorrectFile);
	} else if (type == "fixed-interval") {
		unsigned int interval = atoi(argv[6]);
		string order = string(argv[7]);
		unsigned int numUnits = atoi(argv[8]);
		string correctOutFile = string(argv[9]);
		cout << "Fixed-Interval Test (" << interval << "," << order << "," << numUnits << "): " << testInputFile << endl;
		testSP_CNN_fixed(gene, input, cnnM, cnnN, &testIn, interval,  order, numUnits, correctOutFile);
	} else if (type == "early-finish") {
		unsigned int interval = atoi(argv[6]);
		string order = string(argv[7]);
		unsigned int numUnits = atoi(argv[8]);
		string correctOutFile = string(argv[9]);
		cout << "Early-Finish Test (" << interval << "," << order << "," << numUnits << "): " << testInputFile << endl;
		testSP_CNN_early_finish(gene, input, cnnM, cnnN, &testIn, interval,  order, numUnits, correctOutFile);
	} else if (type == "naive-no-share") {
		cout << "Naive (No-Boundary): " << testInputFile << endl;
		string naiveCorrectFile = string(argv[6]);
		testSP_CNN_naive_no_share(gene, input, cnnM, cnnN, &testIn, naiveCorrectFile);
	} else if (type == "fixed-interval-fast") {
		unsigned int interval = atoi(argv[6]);
		string order = string(argv[7]);
		unsigned int numUnits = atoi(argv[8]);
		string correctOutFile = string(argv[9]);
		cout << "Fixed-Interval (Fast) Test (" << interval << "," << order << "," << numUnits << "): " << testInputFile << endl;
		testSP_CNN_fixed_fast_prop(gene, input, cnnM, cnnN, &testIn, interval,  order, numUnits, correctOutFile);
	} else if (type == "early-finish-fast") {
		unsigned int interval = atoi(argv[6]);
		string order = string(argv[7]);
		unsigned int numUnits = atoi(argv[8]);
		string correctOutFile = string(argv[9]);
		cout << "Early-Finish (Fast) Test (" << interval << "," << order << "," << numUnits << "): " << testInputFile << endl;
		testSP_CNN_early_finish_fast_prop(gene, input, cnnM, cnnN, &testIn, interval,  order, numUnits, correctOutFile);
	} else if (type == "fixed-interval-conv") {
		unsigned int interval = atoi(argv[6]);
		string order = string(argv[7]);
		unsigned int numUnits = atoi(argv[8]);
		string correctOutFile = string(argv[9]);
		cout << "Fixed-Interval (Conv) Test (" << interval << "," << order << "," << numUnits << "): " << testInputFile << endl;
		testSP_CNN_fixed_conv_opt(gene, input, cnnM, cnnN, &testIn, interval,  order, numUnits, correctOutFile);
	} else if (type == "early-finish-conv") {
		unsigned int interval = atoi(argv[6]);
		string order = string(argv[7]);
		unsigned int numUnits = atoi(argv[8]);
		string correctOutFile = string(argv[9]);
		cout << "Early-Finish Test  (Conv) (" << interval << "," << order << "," << numUnits << "): " << testInputFile << endl;
		testSP_CNN_early_finish_conv_opt(gene, input, cnnM, cnnN, &testIn, interval,  order, numUnits, correctOutFile);
	}


	delete gene;
	delete input;
	return 0;
}
