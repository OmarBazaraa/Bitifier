#pragma once
#include <vector>
using namespace std;

typedef unsigned char uchar;

class BitsConcatenator
{
private:
	int bytesIdx = 0;
	int sizesIdx = 0;
	vector<int> compressedSizes;
	vector<unsigned char> compressedBytes;

	// ==============================================================================
	//
	// Concatenation functions
	//
public:
	void concatenate(const vector<int>& data, vector<uchar>& outputData);

private:
	void encodeMetaData();

	void encodeToBase256(int number);

	// ==============================================================================
	//
	// Deconcatenation functions
	//
public:
	void deconcatenate(const vector<uchar>& data, vector<int>& outputData);

private:
	void decodeMetaData();

	int decodeFromBase256();
};