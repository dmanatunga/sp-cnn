#ifndef PARTITION_STREAM_H
#define PARTITION_STREAM_H

#include<vector>
#include<stdlib.h>

class Partition {
private:
	size_t _indR1;
	size_t _indR2;
	size_t _indC1;
    size_t _indC2;

    unsigned int _idX;
    unsigned int _idY;

public:
    Partition(unsigned int X, unsigned int Y, size_t R, size_t C, size_t Rdim, size_t Cdim)
    {
      _indR1 = R;
      _indC1 = C;
      _indR2 = R + Rdim - 1;
      _indC2 = C + Cdim - 1;
      _idX = X;
      _idY = Y;
    }

    size_t indR1() { return _indR1; }
    size_t indR2() { return _indR2; }
    size_t indC1() { return _indC1; }
    size_t indC2() { return _indC2; }
    unsigned int idX() { return _idX; }
    unsigned int idY() { return _idY; }
};

class PartitionStream {
private:
	std::vector<Partition> _stream;
	unsigned int _curPos;
	unsigned int _numPartitions;
	unsigned int _dimX;
	unsigned int _dimY;

public:
	PartitionStream(unsigned int numRows, unsigned int numCols)
	{
		_curPos = 0;
		_numPartitions = 0;
		_dimX = numRows;
		_dimY = numCols;
	}

	void addPartition(unsigned int X, unsigned int Y,  size_t R, size_t C, size_t Rdim, size_t Cdim)
	{
		_stream.push_back(Partition(X, Y, R, C, Rdim, Cdim));
		_numPartitions++;
	}

	Partition getNextPartition()
	{
		Partition curPar = _stream[_curPos];
		if (_curPos == (_numPartitions -1))
			_curPos = 0;
		else
			_curPos++;

		return curPar;
	}

	Partition getPartition(unsigned int i)
	{
		return _stream[i];
	}

	unsigned int numPartitions() { return _numPartitions; }
	unsigned int dimX() { return _dimX; }
	unsigned int dimY() { return _dimY; }
};



#endif // PARTITION_STREAM_H
