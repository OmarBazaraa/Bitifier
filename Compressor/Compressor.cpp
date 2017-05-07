#include "Compressor.h"

//
// Compression functions
//

void Compressor::compress(const cv::Mat& imageMat, vector<uchar>& outputBytes) {
	// Clear previous records
	this->compressedData.clear();
	this->concatenatedData.clear();
	this->shapes.clear();
	this->imageBlocks.clear();

	// Pass data to compressor object
	this->imageMat = imageMat;

	// Encode image
	encodeAdvanced();

	// Concatenate compressed data bits
	BitConcatenator concat;
	concat.concatenate(this->compressedData, this->concatenatedData);

	// Encode compression meta-data
	encodeMetaData();

	// Encode the compressed image using Huffman encoding algorithm
	Huffman huffman;
	huffman.encode(this->concatenatedData, outputBytes);
}

void Compressor::encodeAdvanced() {
	detectDominantColor();
	detectImageBlocks();

	// Store image rows & cols count
	compressedData.push_back(imageMat.rows);
	compressedData.push_back(imageMat.cols);

	// Encode image distinct shapes
	compressedData.push_back(shapes.size());
	for (int i = 0; i < shapes.size(); ++i) {
		encodeRunLength(shapes[i]);
	}

	// Encode image blocks info
	int prv = 0;
	sort(imageBlocks.begin(), imageBlocks.end());
	for (int i = 0; i < imageBlocks.size(); ++i) {
		compressedData.push_back(imageBlocks[i].first - prv);
		compressedData.push_back(imageBlocks[i].second);
		prv = imageBlocks[i].first;
	}
}

void Compressor::detectDominantColor() {
	int whiteCnt = 0;

	for (int i = 0; i < imageMat.rows; ++i) {
		for (int j = 0; j < imageMat.cols; ++j) {
			whiteCnt += ((int)imageMat.at<uchar>(i, j) > 0);
		}
	}

	dominantColor = (whiteCnt * 2 > imageMat.rows * imageMat.cols ? 255 : 0);
	blockColor = 255 - dominantColor;
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
	compressedData.push_back(img.rows);
	compressedData.push_back(img.cols);

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
				compressedData.push_back(cnt);
				cnt = 1;
				prv = pixel;
			}
		}
	}
	compressedData.push_back(cnt);
}

void Compressor::encodeMetaData() {
	// Encode compression configuration
	concatenatedData.push_back(dominantColor == 255 ? 1 : 0);
}

// ==============================================================================
//
// Extraction functions
//

void Compressor::extract(vector<uchar>& compressedBytes, cv::Mat& outputImage) {
	// Clear previous records
	dataIdx = 0;
	this->compressedData.clear();
	this->concatenatedData.clear();
	this->shapes.clear();
	this->imageBlocks.clear();
	
	// Decode huffman encoded data and pass it to compressor object
	Huffman huffman;
	huffman.decode(compressedBytes, this->concatenatedData);

	// Retrieve compression meta-data
	decodeMetaData();

	// De-concatenate compressed data bits
	BitConcatenator concat;
	concat.deconcatenate(this->concatenatedData, this->compressedData);

	// Decode image
	decodeAdvanced();

	// Pass image to function caller
	outputImage = this->imageMat;
}

void Compressor::decodeAdvanced() {
	// Retrieve image rows & cols count
	int rows = compressedData[dataIdx++];
	int cols = compressedData[dataIdx++];

	// Retrieve image distinct shapes
	int shapesCnt = compressedData[dataIdx++];
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
	int rows = compressedData[dataIdx++];
	int cols = compressedData[dataIdx++];

	// Retrieve image pixels
	img = cv::Mat(rows, cols, CV_8U);
	int i = 0, j = 0, cnt;
	bool color = true;

	while (i < rows) {
		cnt = compressedData[dataIdx++];

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
	while (dataIdx + 1 < compressedData.size()) {
		int startOffset = compressedData[dataIdx++];
		int shapeIdx = compressedData[dataIdx++];

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
	// Decode compression configuration
	uchar config = concatenatedData.back();
	concatenatedData.pop_back();

	// Retrieve dominant and block colors
	dominantColor = (config == 1 ? 255 : 0);
	blockColor = 255 - dominantColor;
}