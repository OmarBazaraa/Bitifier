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

	encodeSymbols(encodedData);

	// Calculate the prefix sum of symbols frequencies
	symbolsFrqPrefixSum.resize(ALPHA_SIZE);
	symbolsFrqPrefixSum[0] = symbolsFrq[0];
	for (int i = 1; i < ALPHA_SIZE; ++i) {
		symbolsFrqPrefixSum[i] = symbolsFrqPrefixSum[i - 1] + symbolsFrq[i];
	}

	int l = 0;
	int r = 1;

	for (int i = 0; i < ALPHA_SIZE; ++i) {
		if (symbolsFrq[i] == 0)
			continue;

		int w = r - l;

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