#include "BitConcatenator.h"

//
// Concatenation functions
//

void BitConcatenator::concatenate(const vector<int>& data, vector<uchar>& outputData) {
	// Clear previous data
	dataBitStr.clear();
	compressedData.clear();
	compressedDataSizes.clear();

	// Concatenate data
	for (int i = 0; i < data.size(); ++i) {
		encodeBinary(data[i]);
	}
	
	encodeBitString(dataBitStr);
	encodeDataSizes();

	// Swap the two vectors to return concatenated data to function caller
	outputData.swap(compressedData);
}

void BitConcatenator::encodeBitString(const string& str) {
	uchar byte = 0;
	int bitsCount = 8;

	for (int j = 0; j < str.size(); ++j) {
		byte |= (str[j] - '0') << --bitsCount;

		if (bitsCount <= 0) {
			compressedData.push_back(byte);
			byte = 0;
			bitsCount = 8;
		}
	}

	if (bitsCount < 8) {
		compressedData.push_back(byte);
	}
}

void BitConcatenator::encodeBinary(int number) {
	string bitStr = toBinary(number);

	int i;

	for (i = 0; i < blockLengthsCount; ++i)
		if (bitStr.size() <= blockLengths[i])
			break;

	if (i >= blockLengthsCount) {
		throw exception("Bit concatenation failed");
	}

	dataBitStr += bitStr + string(blockLengths[i] - (int)bitStr.size(), '0');
	compressedDataSizes.push_back(i);
}

void BitConcatenator::encodeDataSizes() {
	int cnt = 1;
	int prv = compressedDataSizes.back();
	for (int i = (int)compressedDataSizes.size() - 2; i >= 0; --i) {
		if (prv == compressedDataSizes[i] && cnt < 63) {
			++cnt;
		}
		else {
			cnt |= prv << 6;
			compressedData.push_back(cnt);
			cnt = 1;
			prv = compressedDataSizes[i];
		}
	}
	cnt |= prv << 6;
	compressedData.push_back(cnt);
}

// ==============================================================================
//
// Deconcatenation functions
//

void BitConcatenator::deconcatenate(vector<uchar>& data, vector<int>& outputData) {
	// Clear previous data
	this->dataBitStr.clear();
	this->compressedData.clear();
	this->compressedDataSizes.clear();

	// Concatenate data
	for (int i = 0; i < data.size(); ++i) {
		dataBitStr += byteToBinaryString(data[i]);
	}

	// 
	data.swap(this->compressedData);

	decodeDataSizes();
	decodeBitString(outputData);
}

void BitConcatenator::decodeBitString(vector<int>& outputData) {
	for (int i = 0, l = 0; i < compressedDataSizes.size(); ++i) {
		int dataSize = compressedDataSizes[i];
		int data = toDecimal(dataBitStr.substr(l, dataSize));
		l += dataSize;
		outputData.push_back(data);
	}
}

void BitConcatenator::decodeDataSizes() {
	int bitsCount = 0;

	while (compressedData.size() > (bitsCount + 7) >> 3) {
		uchar data = compressedData.back();
		compressedData.pop_back();

		int cnt = (data & 63);
		int len = blockLengths[(data >> 6)];
		bitsCount += cnt * len;

		for (int i = 0; i < cnt; ++i) {
			compressedDataSizes.push_back(len);
		}
	}

	if (compressedData.size() != (bitsCount + 7) >> 3) {
		throw exception("Could not extract the given file");
	}
}

// ==============================================================================
//
// Helper functions
//

string BitConcatenator::toBinary(int number) {
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

int BitConcatenator::toDecimal(string& binary) {
	int num = 0;

	for (int i = 0; i < binary.size(); ++i) {
		num |= (binary[i] - '0') << i;
	}

	return num;
}

string BitConcatenator::byteToBinaryString(uchar byte) {
	string result;
	for (int i = 7; i >= 0; --i) {
		result += '0' + (bool)(byte & (1 << i));
	}
	return result;
}