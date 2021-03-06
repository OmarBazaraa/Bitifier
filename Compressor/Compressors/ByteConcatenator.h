#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <map>

using namespace std;

typedef unsigned char uchar;

class ByteConcatenator
{
private:
	int bytesIdx = 0;
	int sizesIdx = 0;
	vector<int> rawData;
	vector<uchar> compressedData;
	vector<int> compressedDataSizes;

	// ==============================================================================
	//
	// Concatenation functions
	//
public:
	void concatenate(vector<int>& data, vector<uchar>& outputData);

private:
	void encodeData();

	void encodeDataSizes();

	void encodeToBase256(int number);

	// ==============================================================================
	//
	// Deconcatenation functions
	//
public:
	void deconcatenate(vector<uchar>& data, vector<int>& outputData);

private:
	void decodeData();

	void decodeDataSizes();

	int decodeFromBase256();
};