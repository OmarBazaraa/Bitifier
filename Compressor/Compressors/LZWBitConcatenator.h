#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>
using namespace std;

typedef unsigned char uchar;

class LZWBitConcatenator
{
private:
	string dataBitStr;

	// ==============================================================================
	//
	// Concatenation functions
	//
public:
	void concatenate(const vector<int>& data, vector<uchar>& outputData);

private:
	void encodeBitString(const string& str, vector<uchar>& outputData);

	// ==============================================================================
	//
	// Deconcatenation functions
	//
public:
	void deconcatenate(const vector<int>& data, vector<int>& outputData);

private:
	void decodeBitString(vector<int>& outputData);

	// ==============================================================================
	//
	// Helper functions
	//
private:
	string toBinary(int number);

	int toDecimal(string& binary);
};