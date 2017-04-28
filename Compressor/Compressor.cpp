#include "Compressor.h"

Compressor::Compressor() {
	
}

Compressor::~Compressor() {
	
}

//
// Compression functions
//

void Compressor::compress(const cv::Mat& imageMat, vector<uchar>& outputBytes) {
	// Clear previous records
	compressedBytes.clear();
	compressedSizes.clear();

	// Store image rows count
	compressedSizes.push_back(encodeToBase256(imageMat.rows));

	// Store image cols count
	compressedSizes.push_back(encodeToBase256(imageMat.cols));

	// Store image pixels
	int cnt = 0;
	bool prv = true;
	for (int i = 0; i < imageMat.rows; ++i) {
		for (int j = 0; j < imageMat.cols; ++j) {
			if (prv == imageMat.at<bool>(i, j)) {
				++cnt;
			}
			else {
				compressedSizes.push_back(encodeToBase256(cnt));
				cnt = 1;
				prv = !prv;
			}
		}
	}
	compressedSizes.push_back(encodeToBase256(cnt));

	// Store compression meta-data
	encodeMetaData();

	// Pass compressed data to function caller
	outputBytes.swap(compressedBytes);
}

void Compressor::encodeMetaData() {
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

int Compressor::encodeToBase256(int number) {
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

	return cnt;
}

//
// Extraction functions
//

void Compressor::extract(vector<uchar>& compressedBytes, cv::Mat& outputImage) {
	// Pass data to compressor object
	this->compressedBytes.swap(compressedBytes);

	// Retrieve compression meta-data
	decodeMetaData();

	int dataIdx = 0, sizeIdx = -1;

	// Retrieve image rows count
	int rows = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];

	// Retrieve image cols count
	int cols = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];

	// Retrieve image pixels
	outputImage = cv::Mat(rows, cols, CV_8U);
	int i = 0, j = 0, cnt;
	bool color = true;

	for (int k = 2; k < compressedSizes.size(); ++k) {
		cnt = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
		dataIdx += compressedSizes[sizeIdx];

		while (cnt--) {
			outputImage.at<uchar>(i, j) = (color ? 255 : 0);

			if (++j >= cols) {
				++i;
				j = 0;
			}
		}

		color = !color;
	}
}

void Compressor::decodeMetaData() {
	compressedSizes.clear();

	int bytesCnt = 0;
	int n = compressedBytes.size();

	while (n >= 0 && n > bytesCnt) {
		int cnt = (compressedBytes[--n] & 63);
		int len = (compressedBytes[n] >> 6) + 1;
		bytesCnt += cnt * len;

		for (int i = 0; i < cnt; ++i) {
			compressedSizes.push_back(len);
		}
	}

	if (n != bytesCnt) {
		throw exception("Could not extract the given file");
	}
}

int Compressor::decodeFromBase256(int idx, int size) {
	idx += size;
	int num = 0;

	while (size--) {
		num <<= 8;
		num |= compressedBytes[--idx];
	}

	return num;
}