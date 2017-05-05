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
	Huffman huffman;
	huffman.encode(this->compressedBytes, outputBytes);
}

void Compressor::encodeAdvanced() {
	detectDominantColor();
	detectImageBlocks();

	// Encode compression configuration
	compressedBytes.push_back(dominantColor);

	// Store image rows & cols count
	encodeToBase256(imageMat.rows);
	encodeToBase256(imageMat.cols);

	// Encode image distinct shapes
	encodeToBase256(shapes.size());
	for (int i = 0; i < shapes.size(); ++i) {
		encodeRunLength(shapes[i]);
	}

	// Encode image blocks info
	int prv = 0;
	sort(imageBlocks.begin(), imageBlocks.end());
	for (int i = 0; i < imageBlocks.size(); ++i) {
		encodeToBase256(imageBlocks[i].first - prv);
		encodeToBase256(imageBlocks[i].second);
		prv = imageBlocks[i].first;
	}
}

void Compressor::detectDominantColor() {
	int colorCnt[2] = {};

	for (int i = 0; i < imageMat.rows; ++i) {
		for (int j = 0; j < imageMat.cols; ++j) {
			bool color = ((int)imageMat.at<uchar>(i, j) > 0);
			++colorCnt[color];
		}
	}

	dominantColor = (colorCnt[0] > colorCnt[1] ? 0 : 255);
	blockColor = (dominantColor > 0 ? 0 : 255);
}

void Compressor::detectImageBlocks() {
	// Clear visited matrix
	visited = cv::Mat::zeros(imageMat.rows, imageMat.cols, CV_8U);

	// Scan common shapes
	for (int i = 0; i < imageMat.rows; ++i) {
		for (int j = 0; j < imageMat.cols; ++j) {
			// Continue if previously visited or pixel is of background color
			if (visited.at<bool>(i, j) || imageMat.at<uchar>(i, j) == dominantColor)
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
	visited.at<bool>(row, col) = true;

	// Visit neighbours
	for (int i = 0; i < 8; ++i) {
		int toR = row + dirR[i];
		int toC = col + dirC[i];

		if (valid(toR, toC) && !visited.at<bool>(toR, toC)) {
			dfs(toR, toC);
		}
	}
}

bool Compressor::valid(int row, int col) {
	return (
		row >= 0 && row < imageMat.rows &&
		col >= 0 && col < imageMat.cols && 
		imageMat.at<uchar>(row, col) == blockColor
	);
}

void Compressor::encodeRunLength(const cv::Mat& img) {
	// Store image rows & cols count
	encodeToBase256(img.rows);
	encodeToBase256(img.cols);

	// Store image pixels
	int cnt = 0;
	bool pixel, prv = true;
	for (int i = 0; i < img.rows; ++i) {
		for (int j = 0; j < img.cols; ++j) {
			pixel = (img.at<uchar>(i, j) == dominantColor);

			if (prv == pixel) {
				++cnt;
			}
			else {
				encodeToBase256(cnt);
				cnt = 1;
				prv = pixel;
			}
		}
	}
	encodeToBase256(cnt);
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

void Compressor::encodeToBase256(int number) {
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

//
// Extraction functions
//

void Compressor::extract(vector<uchar>& compressedBytes, cv::Mat& outputImage) {
	// Clear previous records
	bytesIdx = sizesIdx = 0;
	this->compressedBytes.clear();
	this->compressedSizes.clear();
	this->shapes.clear();
	this->imageBlocks.clear();
	
	// Decode huffman encoded data and pass it to compressor object
	Huffman huffman;
	huffman.decode(compressedBytes, this->compressedBytes);

	// Retrieve compression meta-data
	decodeMetaData();

	// Decode image
	decodeAdvanced();

	outputImage = this->imageMat;
}

void Compressor::decodeAdvanced() {
	// Retrieve compression configurations
	dominantColor = compressedBytes[bytesIdx++];
	blockColor = (dominantColor > 0 ? 0 : 255);

	// Retrieve image rows & cols count
	int rows = decodeFromBase256();
	int cols = decodeFromBase256();

	// Retrieve image distinct shapes
	int shapesCnt = decodeFromBase256();
	for (int i = 0; i < shapesCnt; ++i) {
		cv::Mat shape;
		decodeRunLength(shape);
		shapes.push_back(shape);
	}

	imageMat = cv::Mat(rows, cols, CV_8U, cv::Scalar(dominantColor));
	decodeImageBlocks();
}

void Compressor::decodeRunLength(cv::Mat& img) {
	// Retrieve image rows & cols count
	int rows = decodeFromBase256();
	int cols = decodeFromBase256();

	// Retrieve image pixels
	img = cv::Mat(rows, cols, CV_8U);
	int i = 0, j = 0, cnt;
	bool color = true;

	while (i < rows) {
		cnt = decodeFromBase256();

		while (cnt--) {
			img.at<uchar>(i, j) = (color ? dominantColor : blockColor);

			if (++j >= cols) {
				++i;
				j = 0;
			}
		}

		color = !color;
	}
}

void Compressor::decodeImageBlocks() {
	int startIdx = 0;

	// Retrieve image blocks info
	while (sizesIdx + 1 < compressedSizes.size()) {
		int startOffset = decodeFromBase256();
		int shapeIdx = decodeFromBase256();

		imageBlocks.push_back({ startOffset, shapeIdx });

		startIdx += startOffset;
		int startRow = startIdx / imageMat.cols;
		int startCol = startIdx % imageMat.cols;

		for (int i = 0; i < shapes[shapeIdx].rows; ++i) {
			for (int j = 0; j < shapes[shapeIdx].cols; ++j) {
				imageMat.at<uchar>(startRow + i, startCol + j) = shapes[shapeIdx].at<uchar>(i, j);
			}
		}
	}
}

void Compressor::decodeMetaData() {
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

int Compressor::decodeFromBase256() {
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