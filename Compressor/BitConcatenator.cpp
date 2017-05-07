#include "BitConcatenator.h"

//
// Concatenation functions
//

void BitConcatenator::concatenate(vector<int>& data, vector<uchar>& outputData) {
	// Clear previous data
	rawData.clear();
	compressedData.clear();
	compressedDataSizes.clear();

	// 
	data.swap(rawData);

	// Concatenate data
	encodeData();
	encodeDataSizes();

	// Swap the two vectors to return concatenated data to function caller
	outputData.swap(compressedData);
}

void BitConcatenator::encodeData() {
	for (int i = 0; i < rawData.size(); ++i) {
		encodeToBase256(rawData[i]);
	}
}

void BitConcatenator::encodeDataSizes() {
	int cnt = 1;
	int prv = compressedDataSizes.back();
	for (int i = (int)compressedDataSizes.size() - 2; i >= 0; --i) {
		if (prv == compressedDataSizes[i] && cnt < 63) {
			++cnt;
		}
		else {
			cnt |= (prv - 1) << 6;
			compressedData.push_back(cnt);
			cnt = 1;
			prv = compressedDataSizes[i];
		}
	}
	cnt |= (prv - 1) << 6;
	compressedData.push_back(cnt);
}

void BitConcatenator::encodeToBase256(int number) {
	int cnt = 0;

	if (number == 0) {
		compressedData.push_back(number);
		++cnt;
	}

	while (number > 0) {
		compressedData.push_back(number);
		number >>= 8;	// divide 256
		++cnt;
	}

	compressedDataSizes.push_back(cnt);
}

// ==============================================================================
//
// Deconcatenation functions
//

void BitConcatenator::deconcatenate(vector<uchar>& data, vector<int>& outputData) {
	// Clear previous data
	bytesIdx = sizesIdx = 0;
	rawData.clear();
	compressedData.clear();
	compressedDataSizes.clear();

	// 
	data.swap(compressedData);

	// De-concatenate data
	decodeDataSizes();
	decodeData();

	// Swap the two vectors to return concatenated data to function caller
	outputData.swap(rawData);
}

void BitConcatenator::decodeData() {
	for (int i = 0; i < compressedDataSizes.size(); ++i) {
		rawData.push_back(decodeFromBase256());
	}
}

void BitConcatenator::decodeDataSizes() {
	int bytesCnt = 0;

	while (compressedData.size() > bytesCnt) {
		uchar data = compressedData.back();
		compressedData.pop_back();

		int cnt = (data & 63);
		int len = (data >> 6) + 1;
		bytesCnt += cnt * len;

		for (int i = 0; i < cnt; ++i) {
			compressedDataSizes.push_back(len);
		}
	}

	if (compressedData.size() != bytesCnt) {
		throw exception("Could not extract the given file");
	}
}

int BitConcatenator::decodeFromBase256() {
	int size = compressedDataSizes[sizesIdx++];
	int idx = size + bytesIdx;
	int num = 0;

	bytesIdx += size;

	while (size--) {
		num <<= 8;
		num |= compressedData[--idx];
	}

	return num;
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