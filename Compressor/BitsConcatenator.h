#pragma once
#include <vector>
#include <stack>
using namespace std;

typedef unsigned char uchar;

class BitsConcatenator
{
private:
	string dataBitStr;
	string dataSizesBitStr;

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

	// ==============================================================================
	//
	// Deconcatenation functions
	//
public:
	void deconcatenate(const vector<uchar>& data, vector<int>& outputData);

private:
	void decodeBitString(vector<int>& outputData);

	int decodeDataSizes();

	// ==============================================================================
	//
	// Helper functions
	//
private:
	string toBinary(int number);

	int toDecimal(string& binary);
};