#include "BitConcatenatorBeta.h"

//
// Concatenation functions
//

void BitConcatenatorBeta::concatenate(const vector<int>& data, vector<uchar>& outputData) {
	// Clear previous data
	dataBitStr.clear();
	dataSizesBitStr.clear();
	compressedBytes.clear();

	// Concatenate data
	for (int i = 0; i < data.size(); ++i) {
		encodeBinary(data[i]);
	}

	encodeBitString(dataBitStr + dataSizesBitStr);

	// Swap the two vectors to return concatenated data to function caller
	outputData.swap(compressedBytes);
}

void BitConcatenatorBeta::encodeBitString(const string& str) {
	uchar byte = 0;
	int bitsCount = 0;

	for (int i = 0; i < dataBitStr.size(); ++i) {
		byte |= (dataBitStr[i] - '0') << bitsCount++;

		if (bitsCount >= 8) {
			compressedBytes.push_back(byte);
			byte = 0;
			bitsCount = 0;
		}
	}

	if (bitsCount > 0) {
		compressedBytes.push_back(byte);
	}
}

void BitConcatenatorBeta::encodeBinary(int number) {
	string bitStr = toBinary(number);
	string bitsCountStr = toBinary(bitStr.size() - 1);

	if (bitsCountStr.size() > 5) {
		throw exception("Bits concatenation fialed");
	}

	dataBitStr += bitStr;
	dataSizesBitStr += bitsCountStr + string(5 - bitsCountStr.size(), '0');
}

// ==============================================================================
//
// Deconcatenation functions
//

void BitConcatenatorBeta::deconcatenate(const vector<uchar>& data, vector<int>& outputData) {
	// Clear previous data
	dataBitStr.clear();
	dataSizesBitStr.clear();
	compressedBytes.clear();

	// Concatenate data
	for (int i = 0; i < data.size(); ++i) {
		dataBitStr += toBinary(data[i]);
	}

	decodeDataSizes();
	decodeBitString(outputData);
}

void BitConcatenatorBeta::decodeBitString(vector<int>& outputData) {
	int i = 0;

	while (!dataSizes.empty()) {
		int size = dataSizes.top();
		int data = toDecimal(dataBitStr.substr(i, size));
		outputData.push_back(data);
		dataSizes.pop();
		i += size;
	}
}

void BitConcatenatorBeta::decodeDataSizes() {
	int n = dataBitStr.size();
	int dataBitsCount = 0;

	while (n > dataBitsCount) {
		int size = toDecimal(dataBitStr.substr(n - 5));
		dataBitsCount += size;
		dataSizes.push(size);
		dataBitStr.erase(n -= 5, n);
	}
}

// ==============================================================================
//
// Helper functions
//

string BitConcatenatorBeta::toBinary(int number) {
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

int BitConcatenatorBeta::toDecimal(string& binary) {
	int num = 0;

	for (int i = 0; i < binary.size(); ++i) {
		num |= (binary[i] - '0') << i;
	}

	return num;
}