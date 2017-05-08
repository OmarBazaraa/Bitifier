#include "LZWBitConcatenator.h"

//
// Concatenation functions
//

void LZWBitConcatenator::concatenate(vector<int>& data, vector<uchar>& outputData) {
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

void LZWBitConcatenator::encodeData() {
	for (int i = 0; i < rawData.size(); ++i) {
		encodeToBase256(rawData[i]);
	}
}

void LZWBitConcatenator::encodeDataSizes() {
	int cnt = 1;
	int prv = compressedDataSizes.back();
	for (int i = (int)compressedDataSizes.size() - 2; i >= 0; --i) {
		if (prv == compressedDataSizes[i] && cnt < 127) {
			++cnt;
		}
		else {
			cnt |= (prv - 1) << 7;
			compressedData.push_back(cnt);
			cnt = 1;
			prv = compressedDataSizes[i];
		}
	}
	cnt |= (prv - 1) << 7;
	compressedData.push_back(cnt);
}

void LZWBitConcatenator::encodeToBase256(int number) {
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

void LZWBitConcatenator::deconcatenate(vector<uchar>& data, vector<int>& outputData) {
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

void LZWBitConcatenator::decodeData() {
	for (int i = 0; i < compressedDataSizes.size(); ++i) {
		rawData.push_back(decodeFromBase256());
	}
}

void LZWBitConcatenator::decodeDataSizes() {
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

int LZWBitConcatenator::decodeFromBase256() {
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