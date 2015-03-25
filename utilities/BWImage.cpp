#include "BWImage.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>
#include <iostream>

using namespace std;

BWImage::BWImage() 
{
	_image = NULL;
	_R = 0;
	_C = 0;
}

BWImage::BWImage(size_t R, size_t C) 
{
	_R = R;
	_C = C;
	_image = new float[_R * _C];
}

BWImage::~BWImage()
{
	if (_image != NULL)
		delete[] _image;
}

void BWImage::dlmread(string filename, string delimiter)
{

	ifstream dlmfile(filename.c_str(), ifstream::in);
	assert(_image == NULL);
	assert(!dlmfile.fail());
	vector<float> elements;
	string line;
	float val;

	if (getline(dlmfile, line)) {
		_R = 1;
		size_t start = 0, end = 0;

		while (end != string::npos)
		{
			float val;
			end = line.find(delimiter, start);

			string elem = line.substr(start, (end == string::npos) ? string::npos : end - start);
			istringstream(elem) >> val;
			elements.push_back(val);

			start = end + delimiter.size();
		}
		_C = elements.size();

		while (getline(dlmfile, line)) {
			_R++;
			size_t start = 0, end = 0;
			while (end != string::npos)
			{
				float val;
				end = line.find(delimiter, start);

				string elem = line.substr(start, (end == string::npos) ? string::npos : end - start);
				istringstream stream(elem);
				stream >> val;
				elements.push_back(val);

				start = end + delimiter.size();
			}
		}

		assert(elements.size() == (_R * _C));
		_image = new float[_R * _C];
		for (size_t i = 0; i < _R * _C; i++) {
			_image[i] = elements[i];
		}
		dlmfile.close();
	} else {
		// Throw error since file empty
	}
}


void BWImage::dlmwrite(string filename, string delimiter)
{
	ofstream dlmfile(filename.c_str(), ofstream::out);

	size_t i, j;
	for (i = 0; i < _R; i++) {
		for (j = 0; j < _C-1; j++) {
			dlmfile << _image[i * _C + j] << delimiter;

		}
		dlmfile << _image[i * _C + j] << endl;
	}

	dlmfile.close();
}

float* BWImage::getImage()
{
	return _image;
}

bool BWImage::compare(BWImage *img) 
{
	if ((img->R() != _R) || (img->C() != _C))
		return false;

	float* cmpImg = img->getImage();
	for (size_t i = 0; i < _R; i++) {
		for (size_t j = 0; j < _C; j++) {
			float val1 = _image[i * _C + j];
			float val2 = cmpImg[i * _C + j];
			if (abs(val1 - val2) >= THRESHOLD)
				return false;
		}
	}
	return true;
}

unsigned int BWImage::numPixelsDiff(BWImage *img)
{
	if (img->R() != _R || img->C() != _C) {
		cout << "Input Image (" << img->R() << "," << img->C() << ") ";
		cout << " does not match (" << _R << "," << _C << ")" << endl;
		assert(img->R() == _R);
		assert(img->C() == _C);
	}

	unsigned int numDiff = 0;

	float* cmpImg = img->getImage();
	for (size_t i = 0; i < _R; i++) {
		for (size_t j = 0; j < _C; j++) {
			float val1 = _image[i * _C + j];
			float val2 = cmpImg[i * _C + j];
			if (abs(val1 - val2) >= THRESHOLD) {
				numDiff++;
			}
		}
	}
	return numDiff;
}


