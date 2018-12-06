#include "LZWBitConcatenator.h"

//
// Concatenation functions
//

void LZWBitConcatenator::concatenate(const vector<int>& data, vector<uchar>& outputData) {
	// Clear previous data
	dataBitStr.clear();

	// Concatenate data
	for (int i = 0; i < data.size(); ++i) {
		string bitStr = toBinary(data[i]);
		dataBitStr += bitStr + string(14 - (int)bitStr.size(), '0');
	}

	encodeBitString(dataBitStr, outputData);
}

void LZWBitConcatenator::encodeBitString(const string& str, vector<uchar>& outputData) {
	int byte = 0;
	int bitsCount = 0;

	for (int i = 0; i < str.size(); ++i) {
		byte |= (str[i] - '0') << bitsCount++;

		if (bitsCount >= 8) {
			outputData.push_back(byte);
			byte = 0;
			bitsCount = 0;
		}
	}

	if (bitsCount > 0) {
		outputData.push_back(byte);
	}
}

// ==============================================================================
//
// Deconcatenation functions
//

void LZWBitConcatenator::deconcatenate(const vector<int>& data, vector<int>& outputData) {
	// Clear previous data
	dataBitStr.clear();

	// Concatenate data
	for (int i = 0; i < data.size(); ++i) {
		string bitStr = toBinary(data[i]);
		dataBitStr += bitStr + string(32 - (int)bitStr.size(), '0');
	}

	decodeBitString(outputData);
}

void LZWBitConcatenator::decodeBitString(vector<int>& outputData) {
	int l = 0;

	while (l + 14 <= dataBitStr.size()) {
		string binary = dataBitStr.substr(l, 14);
		l += 14;
		outputData.push_back(toDecimal(binary));
	}
}

// ==============================================================================
//
// Helper functions
//

string LZWBitConcatenator::toBinary(int number) {
	if (number == 0) {
		return "0";
	}

	string bitStr;

	while (number > 0) {
		bitStr += '0' + (number & 1);
		number >>= 1;
	}

	return bitStr;
}

int LZWBitConcatenator::toDecimal(string& binary) {
	int num = 0;

	for (int i = 0; i < binary.size(); ++i) {
		num |= (binary[i] - '0') << i;
	}

	return num;
}