#include<cmath>
#include<vector>

#include "PartitionGenerator.h"

PartitionStream get_partition_stream(std::string type, size_t M, size_t N, size_t cnnR, size_t cnnC)
{
	if (type == "row-major") {
		return row_major_stream(M, N, cnnR, cnnC);
	} else if (type == "col-major") {
		return col_major_stream(M, N, cnnR, cnnC);
	} else if (type == "rev-row-major") {
		return rev_row_major_stream(M, N, cnnR, cnnC);
	} else {
		// Throw error for invalid exception
	}
}

PartitionStream row_major_stream(size_t M, size_t N, size_t cnnR, size_t cnnC)
{
	unsigned int num_row_segs = (M + cnnR - 1) / cnnR;
	unsigned int num_col_segs = (N + cnnC - 1) / cnnC;
	unsigned int num_partitions = num_row_segs * num_col_segs;

	PartitionStream stream(num_row_segs, num_col_segs);
	size_t indR = 0;
	for (unsigned int i = 0; i < num_row_segs; i++) {
		size_t indC = 0;
		for (unsigned int j = 0; j < num_col_segs; j++) {
			stream.addPartition(i, j, indR, indC, cnnR, cnnC);
			indC += cnnC;
		}
		indR += cnnR;
	}
	return stream;
}

PartitionStream col_major_stream(size_t M, size_t N, size_t cnnR, size_t cnnC)
{
	unsigned int num_row_segs = (M + cnnR - 1) / cnnR;
	unsigned int num_col_segs = (N + cnnC - 1) / cnnC;
	unsigned int num_partitions = num_row_segs * num_col_segs;

	PartitionStream stream(num_row_segs, num_col_segs);

	size_t indC = 0;
	for (unsigned int j = 0; j < num_col_segs; j++) {
		size_t indR = 0;
		for (unsigned int i = 0; i < num_row_segs; i++) {
			stream.addPartition(i, j, indR, indC, cnnR, cnnC);
			indR += cnnR;
		}
		indC += cnnC;
	}
	return stream;
}

PartitionStream rev_row_major_stream(size_t M, size_t N, size_t cnnR, size_t cnnC)
{
	unsigned int num_row_segs = (M + cnnR - 1) / cnnR;
	unsigned int num_col_segs = (N + cnnC - 1) / cnnC;
	unsigned int num_partitions = num_row_segs * num_col_segs;

	PartitionStream stream(num_row_segs, num_col_segs);
	size_t indR = 0;
	for (unsigned int i = 0; i < num_row_segs; i++) {
		size_t indC = N - cnnC;
		for (int j = num_col_segs - 1; j >= 0; j--) {
			stream.addPartition(i, j, indR, indC, cnnR, cnnC);
			indC -= cnnC;
		}
		indR += cnnR;
	}
	return stream;
}
