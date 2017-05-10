#pragma once
#include <iostream>
#include <vector>
#include <stack>

#include "Huffman.h"
using namespace std;

typedef unsigned char uchar;

class BitConcatenator
{
private:
	string dataBitStr;
	string dataSizesBitStr;

	vector<uchar> dataSizesLengths;

	vector<uchar> compressedBytes;
	stack<int> dataSizes;

	// ==============================================================================
	//
	// Concatenation functions
	//
public:
	void concatenate(const vector<int>& data, vector<uchar>& outputData);

private:
	void encodeBitString(const string& str);

	void encodeBinary(int number);

	void encodeMetaData();

	// ==============================================================================
	//
	// Deconcatenation functions
	//
public:
	void deconcatenate(const vector<uchar>& data, vector<int>& outputData);

private:
	void decodeBitString(vector<int>& outputData);

	void decodeDataSizes();

	// ==============================================================================
	//
	// Helper functions
	//
private:
	string toBinary(int number);

	int toDecimal(string& binary);
};