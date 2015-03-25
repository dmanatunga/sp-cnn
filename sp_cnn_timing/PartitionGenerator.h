/*
 * PartitionGenerator.h
 *
 *  Created on: Apr 10, 2014
 *      Author: manatunga
 */

#ifndef PARTITION_GENERATOR_H_
#define PARTITION_GENERATOR_H_

#include<string>
#include "PartitionStream.h"

PartitionStream get_partition_stream(std::string type, size_t M, size_t N, size_t cnnR, size_t cnnC);
PartitionStream row_major_stream(size_t M, size_t N, size_t cnnR, size_t cnnC);
PartitionStream col_major_stream(size_t M, size_t N, size_t cnnR, size_t cnnC);
PartitionStream rev_row_major_stream(size_t M, size_t N, size_t cnnR, size_t cnnC);

#endif /* PARTITION_GENERATOR_H_ */
