#include <string>
#include <iostream>
#include "utilities/BWImage.h"
#include "cnn/cnn.h"
#include "SP_CNN.h"
using namespace std;




int main(int argc, char* argv[])
{
	int num_args_needed = 7;
	if (argc != num_args_needed)
	{
		// Error on necessary parameters
		cerr << "Invalid number of arguments. Expected " << num_args_needed << " arguments. Received " << argc << " arguments." << endl;
		return -1;
	}

	unsigned int cnnM = atoi(argv[1]);
	unsigned int cnnN = atoi(argv[2]);
	string geneName = string(argv[3]);
	string testInputFile = string(argv[4]);
	string correctOutFile = string(argv[5]);
	string type = string(argv[6]);

	CNNGene* gene = CNNGeneFactory::createCNNGene(geneName);

	BWImage testIn, correctOutput;
	testIn.dlmread(testInputFile, " ");
	correctOutput.dlmread(correctOutFile, " ");
	CNNInput* input = gene->getInput(testIn.getImage(), testIn.R(), testIn.C());

	SP_CNN sp_cnn(testIn.R(), testIn.C(),  gene->r(), cnnM, cnnN, 1);

	SP_CNN::RunData runData;
	if (type == "share") {
		runData = sp_cnn.naiveRun(gene, input);
	} else if (type == "no-share") {
		runData = sp_cnn.naiveRunNoShareBoundary(gene, testIn.getImage());
	}

	BWImage naiveOut(testIn.R(), testIn.C());
	sp_cnn.copyOutput(naiveOut.getImage());

	cout << geneName << "(" << testInputFile << ",1,0)";
	cout << ": (" << runData.totalConvTime << "," << runData.virtualConvTime;
	unsigned int numPixelsDiff = correctOutput.numPixelsDiff(&naiveOut);
	double percentDiff = ((double)numPixelsDiff) / ((double)(testIn.R() * testIn.C()));
	cout << "," << numPixelsDiff << "," << percentDiff << ")" << endl;

	delete gene;
	delete input;
	return 0;
}
