#include "LZW.h"

//
// Encoding functions
//

void LZW::encode(const vector<uchar>& data, vector<uchar>& outputData) {
	initEncoderDictionary();

	vector<int> result;
	vector<uchar> pattern;

	for (int i = 0; i < data.size(); ++i) {
		pattern.push_back(data[i]);

		if (!encoderDictionary.count(pattern)) {
			encoderDictionary[pattern] = encoderDictionary.size();

			pattern.pop_back();
			result.push_back(encoderDictionary[pattern]);

			pattern.clear();
			pattern.push_back(data[i]);
		}
	}

	result.push_back(encoderDictionary[pattern]);

	BitConcatenator concat;
	concat.concatenate(result, outputData);
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

void LZW::decode(vector<uchar>& data, vector<uchar>& outputData) {
	initDecoderDictionary();

	vector<int> result;
	vector<uchar> prv, entry;

	BitConcatenator concat;
	concat.deconcatenate(data, result);

	for (int i = 0; i < result.size(); ++i) {
		int code = result[i];

		if (code > decoderDictionary.size()) {
			throw exception("LZW decoding failed");
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

		outputData.insert(outputData.end(), entry.begin(), entry.end());
		prv = entry;
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