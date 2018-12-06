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
	int blockLengthsCount = 4;
	int blockLengths[4] = { 4, 8, 12, 32 };

	string dataBitStr;
	vector<uchar> compressedData;
	vector<int> compressedDataSizes;

	// ==============================================================================
	//
	// Concatenation functions
	//
public:
	void concatenate(const vector<int>& data, vector<uchar>& outputData);

private:
	void encodeBitString(const string& str);

	void encodeBinary(int number);

	void encodeDataSizes();

	// ==============================================================================
	//
	// Deconcatenation functions
	//
public:
	void deconcatenate(vector<uchar>& data, vector<int>& outputData);

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

	string byteToBinaryString(uchar byte);
};