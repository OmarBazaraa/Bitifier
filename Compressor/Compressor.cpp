#include "Compressor.h"

//
// Compression functions
//

void Compressor::compress(const cv::Mat& imageMat, vector<uchar>& outputBytes) {
	// Clear previous records
	this->compressedBytes.clear();
	this->compressedSizes.clear();
	this->shapes.clear();
	this->imageBlocks.clear();

	// Pass data to compressor object
	this->imageMat = imageMat;

	// Encode image
	encodeAdvanced();
	
	// Encode compression meta-data
	encodeMetaData();

	// Encode the compressed image using Huffman encoding algorithm
	// and pass compressed data to function caller
	Huffman huffman;
	huffman.encode(this->compressedBytes, outputBytes);

	// Pass compressed data to function caller
	//outputBytes.swap(this->compressedBytes);
}

void Compressor::encodeAdvanced() {
	// Clear visited matrix
	vis = cv::Mat::zeros(imageMat.rows, imageMat.cols, CV_8U);

	// Scan common shapes
	for (int i = 0; i < imageMat.rows; ++i) {
		for (int j = 0; j < imageMat.cols; ++j) {
			// Continue if previously visited or pixel is white
			if (vis.at<bool>(i, j) || ((int)imageMat.at<uchar>(i, j) > 0))
				continue;

			// Get shape boundries
			minRow = minCol = 1e9;
			maxRow = maxCol = -1e9;
			dfs(i, j);
			cv::Mat shape(imageMat, Range(minRow, maxRow + 1), Range(minCol, maxCol + 1));

			// Store block info
			imageBlocks.push_back({ imageMat.cols * minRow + minCol, storeUniqueShape(shape) });
		}
	}

	// Store image rows & cols count
	compressedSizes.push_back(encodeToBase256(imageMat.rows));
	compressedSizes.push_back(encodeToBase256(imageMat.cols));

	// Encode image distinct shapes
	compressedSizes.push_back(encodeToBase256(shapes.size()));
	for (int i = 0; i < shapes.size(); ++i) {
		encodeRunLength(shapes[i]);
	}

	// Encode image blocks info
	int prv = 0;
	sort(imageBlocks.begin(), imageBlocks.end());
	for (int i = 0; i < imageBlocks.size(); ++i) {
		compressedSizes.push_back(encodeToBase256(imageBlocks[i].first - prv));
		compressedSizes.push_back(encodeToBase256(imageBlocks[i].second));
		prv = imageBlocks[i].first;
	}
}

int Compressor::storeUniqueShape(const cv::Mat& shape) {
	for (int i = 0; i < shapes.size(); ++i) {
		if (shape.size() != shapes[i].size())
			continue;

		if (countNonZero(shape ^ shapes[i]) == 0)
			return i;
	}
	
	shapes.push_back(shape);
	return (int)shapes.size() - 1;
}

void Compressor::dfs(int row, int col) {
	// Get boundries
	minRow = min(minRow, row);
	minCol = min(minCol, col);
	maxRow = max(maxRow, row);
	maxCol = max(maxCol, col);

	// Set current pixel as visisted
	vis.at<bool>(row, col) = true;

	// Visit neighbours
	for (int i = 0; i < 8; ++i) {
		int toR = row + dirR[i];
		int toC = col + dirC[i];

		if (valid(toR, toC) && !vis.at<bool>(toR, toC)) {
			dfs(toR, toC);
		}
	}
}

bool Compressor::valid(int row, int col) {
	return (row >= 0 && row < imageMat.rows && col >= 0 && col < imageMat.cols && !imageMat.at<bool>(row, col));
}

void Compressor::encodeRunLength(const cv::Mat& img) {
	// Store image rows & cols count
	compressedSizes.push_back(encodeToBase256(img.rows));
	compressedSizes.push_back(encodeToBase256(img.cols));

	// Store image pixels
	int cnt = 0;
	bool pixel, prv = true;
	for (int i = 0; i < img.rows; ++i) {
		for (int j = 0; j < img.cols; ++j) {
			pixel = ((int)img.at<uchar>(i, j) > 0);

			if (prv == pixel) {
				++cnt;
			}
			else {
				compressedSizes.push_back(encodeToBase256(cnt));
				cnt = 1;
				prv = pixel;
			}
		}
	}
	compressedSizes.push_back(encodeToBase256(cnt));
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
	// Clear previous records
	this->compressedBytes.clear();
	this->compressedSizes.clear();
	this->shapes.clear();
	this->imageBlocks.clear();
	
	// Decode huffman encoded data and pass it to compressor object
	Huffman huffman;
	huffman.decode(compressedBytes, this->compressedBytes);

	// Pass data to compressor object
	//this->compressedBytes.swap(compressedBytes);

	// Retrieve compression meta-data
	decodeMetaData();

	// Decode image
	decodeAdvanced();

	outputImage = this->imageMat;
}

void Compressor::decodeAdvanced() {
	int dataIdx = 0, sizeIdx = -1;

	// Retrieve image rows & cols count
	int rows = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];
	int cols = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];

	// Retrieve image distinct shapes
	int shapesCnt = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];
	
	for (int i = 0; i < shapesCnt; ++i) {
		cv::Mat shape;
		decodeRunLength(shape, dataIdx, sizeIdx);
		shapes.push_back(shape);
	}

	imageMat = cv::Mat(rows, cols, CV_8U, cv::Scalar(255));
	
	int startIdx = 0;

	// Retrieve image blocks info
	while (sizeIdx + 1 < compressedSizes.size()) {
		int startOffset = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
		dataIdx += compressedSizes[sizeIdx];

		int shapeIdx = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
		dataIdx += compressedSizes[sizeIdx];

		imageBlocks.push_back({ startOffset, shapeIdx });

		startIdx += startOffset;
		int startRow = startIdx / cols;
		int startCol = startIdx % cols;

		for (int i = 0; i < shapes[shapeIdx].rows; ++i) {
			for (int j = 0; j < shapes[shapeIdx].cols; ++j) {
				imageMat.at<uchar>(startRow + i, startCol + j) = shapes[shapeIdx].at<uchar>(i, j);
			}
		}
		/*shapes[idx].copyTo(imageMat(
			Range(row, row + shapes[idx].rows - 1),
			Range(col, col + shapes[idx].cols - 1)
		));*/
	}
}

void Compressor::decodeRunLength(cv::Mat& img, int& dataIdx, int& sizeIdx) {
	// Retrieve image rows & cols count
	int rows = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];
	int cols = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];

	// Retrieve image pixels
	img = cv::Mat(rows, cols, CV_8U);
	int i = 0, j = 0, cnt;
	bool color = true;

	while (i < rows) {
		cnt = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
		dataIdx += compressedSizes[sizeIdx];

		while (cnt--) {
			img.at<uchar>(i, j) = (color ? 255 : 0);

			if (++j >= cols) {
				++i;
				j = 0;
			}
		}

		color = !color;
	}
}

void Compressor::decodeMetaData() {
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