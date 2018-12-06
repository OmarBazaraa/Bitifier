#include "LZW.h"

//
// Encoding functions
//

void LZW::encode(const vector<uchar>& data, vector<int>& outputData) {
	initEncoderDictionary();

	//
	vector<uchar> pattern;

	for (int i = 0; i < data.size(); ++i) {
		pattern.push_back(data[i]);

		if (!encoderDictionary.count(pattern)) {
			encoderDictionary[pattern] = encoderDictionary.size();

			pattern.pop_back();
			outputData.push_back(encoderDictionary[pattern]);

			pattern.clear();
			pattern.push_back(data[i]);
		}
	}
	outputData.push_back(encoderDictionary[pattern]);
}

void LZW::initEncoderDictionary() {
	vector<uchar> temp;
	temp.push_back(0);

	int idx = 0;

	while (idx < 256) {
		encoderDictionary[temp] = idx;
		temp[0] = ++idx;
	}
}

// ==============================================================================
//
// Decoding functions
//

void LZW::decode(const vector<int>& data, vector<int>& outputData) {
	initDecoderDictionary();

	//
	vector<int> tempData;
	LZWBitConcatenator concat;
	concat.deconcatenate(data, tempData);

	vector<uchar> result;
	vector<uchar> prv, entry;

	for (int i = 0; i < tempData.size(); ++i) {
		int code = tempData[i];

		if (code > decoderDictionary.size()) {
			// throw exception("LZW decoding failed");
			cerr << "LZW  decoding failed" << endl;
		}

		entry = decoderDictionary[code];

		if (code == decoderDictionary.size()) {
			prv.push_back(prv[0]);
			decoderDictionary[decoderDictionary.size()] = prv;
			prv.pop_back();
		}
		else if (!prv.empty()) {
			prv.push_back(entry[0]);
			decoderDictionary[decoderDictionary.size()] = prv;
			prv.pop_back();
		}

		result.insert(result.end(), entry.begin(), entry.end());
		prv = entry;
	}

	// Convert uchar array to integer array
	for (int i = 0; i < result.size(); ++i) {
		if (i & 1) {
			outputData.back() |= result[i];
		}
		else {
			outputData.push_back(result[i] << 8);
		}
	}
}

void LZW::initDecoderDictionary() {
	vector<uchar> temp;
	temp.push_back(0);

	int idx = 0;

	while (idx < 256) {
		decoderDictionary[idx] = temp;
		temp[0] = ++idx;
	}
}