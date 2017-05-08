#pragma once
#include <vector>
#include <string>
#include <map>
#include "LZWBitConcatenator.h"
#include "BitConcatenator.h"
using namespace std;

typedef unsigned char uchar;

class LZW
{
private:
	map<vector<uchar>, int> encoderDictionary;
	map<int, vector<uchar>> decoderDictionary;

	// ==============================================================================
	//
	// Encoding functions
	//
public:
	/**
	 *
	 */
	void encode(const vector<uchar>& data, vector<uchar>& outputData);

private:
	/**
	 *
	 */
	void initEncoderDictionary();


	// ==============================================================================
	//
	// Decoding functions
	//
public:
	/**
	 *
	 */
	void decode(vector<uchar>& data, vector<uchar>& outputData);

private:
	/**
	 *
	 */
	void initDecoderDictionary();
};