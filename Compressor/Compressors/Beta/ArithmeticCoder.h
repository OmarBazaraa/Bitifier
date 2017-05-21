#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "../BitConcatenator.h"
using namespace std;

typedef unsigned char uchar;

class ArithmeticCoder
{
private:
	const int ALPHA_SIZE = 256;

	int dataIdx;
	string binaryStr;
	vector<int> symbolsFrq;
	vector<int> symbolsFrqPrefixSum;

	// ==============================================================================
	//
	// Encoding functions
	//
public:	
	/**
	 * Encode the passed data by generating shorter code words for
	 * more frequent symbols
	 */
	void encode(const vector<uchar>& data, vector<uchar>& encodedData);

private:

	void encodeSymbols(vector<uchar>& encodedData);

	void _encode(const vector<uchar>& data);

	// ==============================================================================
	//
	// Decoding functions
	//
public:
	/**
	 * Decode the passed data by retrieving the code word table and mapping each code
	 * to its corresponding symbol
	 */
	void decode(const vector<uchar>& data, vector<uchar>& decodedData);

private:
	void decodeSymbols(const vector<uchar>& data);

	//
	// Helper functions
	//
private:
	string byteToBinaryString(uchar byte);
};