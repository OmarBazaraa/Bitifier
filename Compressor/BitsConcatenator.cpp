#include "BitsConcatenator.h"

//
// Concatenation functions
//

void BitsConcatenator::concatenate(const vector<int>& data, vector<uchar>& outputData) {
	// Clear previous data
	bytesIdx = sizesIdx = 0;
	compressedBytes.clear();
	compressedSizes.clear();

	// Swap the two vectors
	compressedBytes.swap(outputData);

	// Concatenate data
	for (int i = 0; i < data.size(); ++i) {
		encodeToBase256(data[i]);
	}
	encodeMetaData();

	// Swap the two vectors again to return concatenated data to function caller
	compressedBytes.swap(outputData);
}

void BitsConcatenator::encodeMetaData() {
	int cnt = 1;
	int prv = compressedSizes.back();

	for (int i = (int)compressedSizes.size() - 2; i >= 0; --i) {
		if (prv == compressedSizes[i] && cnt < 63) {
			++cnt;
		}
		else {
			cnt |= (prv - 1) << 6;
			compressedBytes.push_back(cnt);
			cnt = 1;
			prv = compressedSizes[i];
		}
	}

	cnt |= (prv - 1) << 6;
	compressedBytes.push_back(cnt);
}

void BitsConcatenator::encodeToBase256(int number) {
	int cnt = 0;

	if (number == 0) {
		compressedBytes.push_back(number);
		++cnt;
	}

	while (number > 0) {
		compressedBytes.push_back(number);
		number >>= 8;	// divide 256
		++cnt;
	}

	compressedSizes.push_back(cnt);
}

// ==============================================================================
//
// Deconcatenation functions
//

void BitsConcatenator::deconcatenate(const vector<uchar>& data, vector<int>& outputData) {

}

void BitsConcatenator::decodeMetaData() {
	int bytesCnt = 0;
	int n = compressedBytes.size();

	// minus 1 byte for configuration, TODO: to be implemented in better way
	while (n >= 0 && n - 1 > bytesCnt) {
		int cnt = (compressedBytes[--n] & 63);
		int len = (compressedBytes[n] >> 6) + 1;
		bytesCnt += cnt * len;

		for (int i = 0; i < cnt; ++i) {
			compressedSizes.push_back(len);
		}
	}

	if (n - 1 != bytesCnt) {
		throw exception("Could not extract the given file");
	}
}

int BitsConcatenator::decodeFromBase256() {
	int size = compressedSizes[sizesIdx++];
	int idx = size + bytesIdx;
	int num = 0;

	bytesIdx += size;

	while (size--) {
		num <<= 8;
		num |= compressedBytes[--idx];
	}

	return num;
}