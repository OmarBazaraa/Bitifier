#include "ArithmeticCoder.h"

//
// Encoding functions
//

void ArithmeticCoder::encode(const vector<uchar>& data, vector<uchar>& encodedData) {
	if (data.empty())
		return;

	// Count the frequency of each symbol in the given data
	symbolsFrq.resize(ALPHA_SIZE);
	for (int i = 0; i < data.size(); ++i) {
		++symbolsFrq[data[i]];
	}

	//encodeSymbols(encodedData);

	// Calculate the prefix sum of symbols frequencies
	symbolsFrqPrefixSum.resize(ALPHA_SIZE + 1);
	symbolsFrqPrefixSum[0] = 0;
	for (int i = 1; i <= ALPHA_SIZE; ++i) {
		symbolsFrqPrefixSum[i] = symbolsFrqPrefixSum[i - 1] + symbolsFrq[i - 1];
	}

	_encode(data);

	//
	uchar byte = 0;
	int bitsCount = 8;

	for (int i = 0; i < binaryStr.size(); ++i) {
		byte |= (binaryStr[i] - '0') << --bitsCount;

		if (bitsCount <= 0) {
			encodedData.push_back(byte);
			byte = 0;
			bitsCount = 8;
		}
	}

	if (bitsCount < 8) {
		encodedData.push_back(byte);
		encodedData.push_back(bitsCount);	// Number of bits to be ignored
	}
	else {
		encodedData.push_back(0);			// Number of bits to be ignored
	}
}

void ArithmeticCoder::_encode(const vector<uchar>& data) {
	int precision = 32;
	unsigned int top = (1LL << precision) - 1;
	unsigned int qtr = top / 4 + 1;
	unsigned int half = 2 * qtr;
	unsigned int qtr3 = 3 * qtr;

	long long low = 0, high = top, opposite_bits = 0, range;

	// Arithmetic encoding
	for (int i : data) {
		range = high - low + 1;
		high = low + (range * symbolsFrqPrefixSum[i + 1]) / data.size() + 1;
		low = low + (range * symbolsFrqPrefixSum[i]) / data.size();

		while (true) {
			if (high < half) {
				binaryStr += '0';
				while (opposite_bits > 0) {
					binaryStr += '1';
					opposite_bits--;
				}
			}
			else if (low >= half) {
				binaryStr += '1';
				while (opposite_bits > 0) {
					binaryStr += '0';
					opposite_bits--;
				}
				low -= half;
				high -= half;
			}
			else if (low >= qtr && high < qtr3) {
				break;
				opposite_bits += 1;
				low -= qtr;
				high -= qtr;
			}
			else {
				break;
			}

			low = 2 * low;
			high = 2 * high + 1;
		}
	}

	// Encoder flush
	opposite_bits++;
	if (low < qtr) {
		binaryStr += '0';
		while (opposite_bits > 0) {
			binaryStr += '1';
			opposite_bits--;
		}
	}
	else {
		binaryStr += '1';
		while (opposite_bits > 0) {
			binaryStr += '0';
			opposite_bits--;
		}
	}
}

void ArithmeticCoder::encodeSymbols(vector<uchar>& encodedData) {
	ByteConcatenator concat;
	vector<uchar> metaData;
	concat.concatenate(symbolsFrq, metaData);

	int n = metaData.size();

	if (n >= 1 << 16) {
		throw exception("Cannot encode arithmetic coding meta-data");
	}

	encodedData.push_back(n);
	encodedData.push_back(n >> 8);
	encodedData.insert(encodedData.end(), metaData.begin(), metaData.end());
}

// ==============================================================================
//
// Decoding functions
//

void ArithmeticCoder::decode(const vector<uchar>& data, vector<uchar>& decodedData) {

}

void ArithmeticCoder::decodeSymbols(const vector<uchar>& data) {
	int n = 2;
	n += data[0];
	n += data[1] << 8;

	dataIdx = n - 1;

	ByteConcatenator concat;
	vector<uchar> metaData(data.begin() + 2, data.begin() + n);
	concat.deconcatenate(metaData, symbolsFrq);

	// Calculate the prefix sum of symbols frequencies
	symbolsFrqPrefixSum.resize(ALPHA_SIZE);
	symbolsFrqPrefixSum[0] = symbolsFrq[0];
	for (int i = 1; i < ALPHA_SIZE; ++i) {
		symbolsFrqPrefixSum[i] = symbolsFrqPrefixSum[i - 1] + symbolsFrq[i];
	}
}

// ==============================================================================
//
// Helper functions
//

string ArithmeticCoder::byteToBinaryString(uchar byte) {
	string result;
	for (int i = 7; i >= 0; --i) {
		result += '0' + (bool)(byte & (1 << i));
	}
	return result;
}