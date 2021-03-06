#include "Compressor.h"

//
// Compression functions
//

void Compressor::compress(const cv::Mat& imageMat, vector<uchar>& outputBytes) {
	// Clear previous records
	dataIdx = 0;
	this->compressedData.clear();
	this->concatenatedData.clear();
	this->shapes.clear();
	this->shapeBlocks.clear();
	this->imageBlocks.clear();
	this->blockShapes.clear();

	// Pass data to compressor object
	this->imageMat = imageMat;

	// Encode image
	encodeAdvanced();

	// Concatenate compressed data bits
	ByteConcatenator concat;
	concat.concatenate(this->compressedData, this->concatenatedData);

	// Encode compression meta-data
	encodeMetaData();

	// Encode the compressed image using Huffman encoding algorithm
	Huffman huffman;
	huffman.encode(this->concatenatedData, outputBytes);
}

void Compressor::encodeAdvanced() {
	// Store image rows & cols count
	compressedData.push_back(imageMat.rows);
	compressedData.push_back(imageMat.cols);

	// Detecting dominant color must come before detecting image blocks
	detectDominantColor();
	detectImageBlocks();

	// Encode shape definition and image blocks indecies
	encodeDistinctShapes();
	encodeImageBlocks();
}

void Compressor::encodeDistinctShapes() {
	vector<int> encodedShapes;

	// Encode image distinct shapes
	compressedData.push_back(shapes.size());
	for (int i = 0; i < shapes.size(); ++i) {
		//
		// Try different run length encoding techniques and pick the better one
		//
		vector<pair<vector<int>, int>> vec(4);

		encodeRunLengthHorizontal(shapes[i], vec[0].first);
		vec[0].second = RUN_LENGTH_HOR;
		encodeRunLengthVertical(shapes[i], vec[1].first);
		vec[1].second = RUN_LENGTH_VER;
		encodeRunLengthSpiral(shapes[i], vec[2].first);
		vec[2].second = RUN_LENGTH_SPIRAL;
		encodeRunLengthZigZag(shapes[i], vec[3].first);
		vec[3].second = RUN_LENGTH_ZIGZAG;

		sort(vec.begin(), vec.end(), cmp);

		encodedShapes.insert(encodedShapes.end(), vec[0].first.begin(), vec[0].first.end());

		if (i & 1)
			compressedData.back() |= vec[0].second << 2;
		else
			compressedData.push_back(vec[0].second);

		// Encode indecies of blocks refering to the i-th shape in relative order
		encodedShapes.push_back(shapeBlocks[i].size());
		for (int j = 0, prv = 0; j < shapeBlocks[i].size(); ++j) {
			encodedShapes.push_back(shapeBlocks[i][j] - prv);
			prv = shapeBlocks[i][j];
		}
	}

	// Insert encoded shapes
	compressedData.insert(compressedData.end(), encodedShapes.begin(), encodedShapes.end());
}

void Compressor::applySymmetry(cv::Mat& img) {
	int n = img.rows / 2;
	int m = img.cols / 2;

	bool horFlag = true;
	bool verFlag = true;
	
	// Check for symmetry around horizontal axis
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < img.cols; ++j) {
			if (img.at<uchar>(i, j) != img.at<uchar>(img.rows - i - 1, j)) {
				horFlag = false;
				break;
			}
		}

		if (!horFlag) {
			break;
		}
	}

	// Check for symmetry around vertical axis
	for (int j = 0; j < m; ++j) {
		for (int i = 0; i < img.rows; ++i) {
			if (img.at<uchar>(i, j) != img.at<uchar>(i, img.cols - j - 1)) {
				verFlag = false;
				break;
			}
		}

		if (!verFlag) {
			break;
		}
	}

	n = img.rows;
	m = img.cols;

	if (horFlag && verFlag) {
		cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> HOR & VER" << endl;
		n = img.rows / 2 + (img.rows & 1);
		m = img.cols / 2 + (img.cols & 1);
	}
	else if (horFlag) {
		cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> HOR" << endl;
		n = img.rows / 2 + (img.rows & 1);
	}
	else if (verFlag) {
		cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> VER" << endl;
		m = img.cols / 2 + (img.cols & 1);
	}
	else {
		return;
	}

	cv::Mat shape(img, Range(0, n), Range(0, m));
	img = shape;
}

void Compressor::encodeImageBlocks() {
	// Encode image blocks starting pixels (upper left pixels)
	for (int i = 0, prv = 0; i < imageBlocks.size(); ++i) {
		compressedData.push_back(imageBlocks[i].first - prv);
		prv = imageBlocks[i].first;// +shapes[imageBlocks[i].second].cols / 1.65;
	}
}

void Compressor::encodeRunLengthHorizontal(const cv::Mat& img, vector<int>& encodedData) {
	// Store image rows & cols count
	encodedData.push_back(img.rows);
	encodedData.push_back(img.cols);

	// Store image pixels
	int dir = 1;
	int initVal = 0;
	int runCnt = 0;
	bool pixel, prvColor = true;

	for (int i = 0; i < img.rows; ++i) {
		for (int j = initVal; j >= 0 && j < img.cols; j += dir) {
			pixel = (img.at<uchar>(i, j) == dominantColor);

			if (prvColor == pixel) {
				++runCnt;
			}
			else {
				encodedData.push_back(runCnt);
				runCnt = 1;
				prvColor = pixel;
			}
		}

		dir = -dir;
		initVal = img.cols - 1 - initVal;
	}
	encodedData.push_back(runCnt);
}

void Compressor::encodeRunLengthVertical(const cv::Mat& img, vector<int>& encodedData) {
	// Store image rows & cols count
	encodedData.push_back(img.rows);
	encodedData.push_back(img.cols);

	// Store image pixels
	int dir = 1;
	int initVal = 0;
	int runCnt = 0;
	bool pixel, prvColor = true;

	for (int j = 0; j < img.cols; ++j) {
		for (int i = initVal; i >= 0 && i < img.rows; i += dir) {
			pixel = (img.at<uchar>(i, j) == dominantColor);

			if (prvColor == pixel) {
				++runCnt;
			}
			else {
				encodedData.push_back(runCnt);
				runCnt = 1;
				prvColor = pixel;
			}
		}

		dir = -dir;
		initVal = img.rows - 1 - initVal;
	}
	encodedData.push_back(runCnt);
}

void Compressor::encodeRunLengthSpiral(const cv::Mat& img, vector<int>& encodedData) {
	// Store image rows & cols count
	encodedData.push_back(img.rows);
	encodedData.push_back(img.cols);

	// Store image pixels
	int i = 0, j = img.cols - 1;
	int up = 0, down = img.rows - 1, left = 0, right = img.cols - 1;
	int cellsVisCount = 0, cellsCount = img.rows * img.cols;
	int dir = 1;
	int dR[4] = { 0, 1, 0, -1 };
	int dC[4] = { 1, 0, -1, 0 };
	int runCnt = 0;
	bool pixel, prvColor = true;

	while (cellsVisCount++ < cellsCount) {
		pixel = (img.at<uchar>(i, j) == dominantColor);

		if (prvColor == pixel) {
			++runCnt;
		}
		else {
			encodedData.push_back(runCnt);
			runCnt = 1;
			prvColor = pixel;
		}

		int toR = i + dR[dir];
		int toC = j + dC[dir];

		if (toR > down) {
			--right;
			dir = (dir == 3) ? 0 : dir + 1;
		}
		else if (toR < up) {
			++left;
			dir = (dir == 3) ? 0 : dir + 1;
		}
		else if (toC > right) {
			++up;
			dir = (dir == 3) ? 0 : dir + 1;
		}
		else if (toC < left) {
			--down;
			dir = (dir == 3) ? 0 : dir + 1;
		}

		i += dR[dir];
		j += dC[dir];
	}

	encodedData.push_back(runCnt);
}

void Compressor::encodeRunLengthZigZag(const cv::Mat& img, vector<int>& encodedData) {
	// Store image rows & cols count
	encodedData.push_back(img.rows);
	encodedData.push_back(img.cols);

	// Store image pixels
	int i = img.rows - 1, j = img.cols - 1;
	int cellsVisCount = 0, cellsCount = img.rows * img.cols;
	int dir = 0;
	int dR[2] = { 1, -1 };
	int dC[2] = { -1, 1 };
	int runCnt = 0;
	bool pixel, prvColor = true;

	while (cellsVisCount++ < cellsCount) {
		pixel = (img.at<uchar>(i, j) == dominantColor);

		if (prvColor == pixel) {
			++runCnt;
		}
		else {
			encodedData.push_back(runCnt);
			runCnt = 1;
			prvColor = pixel;
		}

		i += dR[dir];
		j += dC[dir];

		if (i < 0) {
			i = 0;
			j -= 2;
			dir = 1 - dir;
		}
		else if (j < 0) {
			j = 0;
			i -= 2;
			dir = 1 - dir;
		}
		else if (i >= img.rows) {
			i = img.rows - 1;
			dir = 1 - dir;
		}
		else if (j >= img.cols) {
			j = img.cols - 1;
			dir = 1 - dir;
		}
	}

	encodedData.push_back(runCnt);
}

void Compressor::encodeMetaData() {
	// Encode compression configuration
	concatenatedData.push_back(dominantColor == 255 ? 1 : 0);
}

// ==============================================================================
//
// Compression helper functions
//

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
			int startPixelIdx = imageMat.cols * minRow + minCol;
			int blockShapeIdx = storeUniqueShape(shape);
			imageBlocks.push_back({ startPixelIdx, blockShapeIdx });
		}
	}

	// Sort image blocks in non-decreasing order of start pixels in order to apply relative positioning
	sort(imageBlocks.begin(), imageBlocks.end());

	// Map shapes to their refering image blocks
	shapeBlocks.resize(shapes.size());
	for (int i = 0; i < imageBlocks.size(); ++i) {
		shapeBlocks[imageBlocks[i].second].push_back(i);
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

bool cmp(const pair<vector<int>, int>& lhs, const pair<vector<int>, int>& rhs) {
	return lhs.first.size() < rhs.first.size();
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
	this->shapeBlocks.clear();
	this->imageBlocks.clear();
	this->blockShapes.clear();
	
	// Decode huffman encoded data and pass it to compressor object
	Huffman huffman;
	huffman.decode(compressedBytes, this->concatenatedData);

	// Retrieve compression meta-data
	decodeMetaData();

	// De-concatenate compressed data bits
	ByteConcatenator concat;
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

	imageMat = cv::Mat(rows, cols, CV_8U, cv::Scalar(dominantColor));

	decodeDistinctShapes();
	decodeImageBlocks();
}

void Compressor::decodeDistinctShapes() {
	// Retrieve distinct shapes count
	int shapesCount = compressedData[dataIdx++];
	shapes.resize(shapesCount);
	shapeBlocks.resize(shapesCount);

	// Retrieve shapes encoding type
	int typeBytesCount = (shapesCount + 1) / 2;
	vector<int> shapesEncodingType;
	for (int i = 0; i < typeBytesCount; ++i) {
		int type = compressedData[dataIdx++];
		shapesEncodingType.push_back(type & 3);
		shapesEncodingType.push_back((type >> 2) & 3);
	}

	// Retrieve image distinct shapes
	for (int i = 0; i < shapesCount; ++i) {
		int type = shapesEncodingType[i];

		if (type == RUN_LENGTH_HOR)
			decodeRunLengthHorizontal(shapes[i]);
		else if (type == RUN_LENGTH_VER)
			decodeRunLengthVertical(shapes[i]);
		else if (type == RUN_LENGTH_SPIRAL)
			decodeRunLengthSpiral(shapes[i]);
		else if (type == RUN_LENGTH_ZIGZAG)
			decodeRunLengthZigZag(shapes[i]);
		
		// Retrieve shape's refering blocks
		int blocksCount = compressedData[dataIdx++];
		shapeBlocks[i].resize(blocksCount);
		for (int j = 0, prv = 0; j < blocksCount; ++j) {
			int blockIdx = compressedData[dataIdx++] + prv;
			prv = blockIdx;
			//shapeBlocks[i][j] = blockIdx;
			blockShapes[blockIdx] = i;
		}
	}
}

void Compressor::decodeImageBlocks() {
	int idx = 0, prv = 0;

	// Retrieve image blocks info
	while (dataIdx < compressedData.size()) {
		int startPixelIdx = compressedData[dataIdx++] + prv;
		int blockShapeIdx = blockShapes[idx++];

		//imageBlocks.push_back({ startPixelIdx, blockShapeIdx });

		int startRow = startPixelIdx / imageMat.cols;
		int startCol = startPixelIdx % imageMat.cols;

		for (int i = 0; i < shapes[blockShapeIdx].rows; ++i) {
			for (int j = 0; j < shapes[blockShapeIdx].cols; ++j) {
				imageMat.at<uchar>(startRow + i, startCol + j) = shapes[blockShapeIdx].at<uchar>(i, j);
			}
		}

		prv = startPixelIdx;// +shapes[blockShapeIdx].cols / 1.65;
	}
}

void Compressor::decodeRunLengthHorizontal(cv::Mat& img) {
	// Retrieve image rows & cols count
	int rows = compressedData[dataIdx++];
	int cols = compressedData[dataIdx++];

	// Retrieve image pixels
	img = cv::Mat(rows, cols, CV_8U);
	int dir = 1;
	int initVal = 0;
	int i = 0, j = 0;
	int runCnt;
	bool color = true;

	while (i < rows) {
		runCnt = compressedData[dataIdx++];

		while (runCnt--) {
			img.at<uchar>(i, j) = (color ? dominantColor : blockColor);

			j += dir;

			if (j < 0 || j >= cols) {
				dir = -dir;
				initVal = img.cols - 1 - initVal;

				j = initVal;
				++i;
			}
		}

		color = !color;
	}
}

void Compressor::decodeRunLengthVertical(cv::Mat& img) {
	// Retrieve image rows & cols count
	int rows = compressedData[dataIdx++];
	int cols = compressedData[dataIdx++];

	// Retrieve image pixels
	img = cv::Mat(rows, cols, CV_8U);
	int dir = 1;
	int initVal = 0;
	int i = 0, j = 0;
	int runCnt;
	bool color = true;

	while (j < cols) {
		runCnt = compressedData[dataIdx++];

		while (runCnt--) {
			img.at<uchar>(i, j) = (color ? dominantColor : blockColor);

			i += dir;

			if (i < 0 || i >= rows) {
				dir = -dir;
				initVal = img.rows - 1 - initVal;

				i = initVal;
				++j;
			}
		}

		color = !color;
	}
}

void Compressor::decodeRunLengthSpiral(cv::Mat& img) {
	// Retrieve image rows & cols count
	int rows = compressedData[dataIdx++];
	int cols = compressedData[dataIdx++];
	
	// Retrieve image pixels
	img = cv::Mat(rows, cols, CV_8U);
	
	int i = 0, j = img.cols - 1;
	int up = 0, down = img.rows - 1, left = 0, right = img.cols - 1;
	int cellsVisCount = 0, cellsCount = img.rows * img.cols;
	int dir = 1;
	int dR[4] = { 0, 1, 0, -1 };
	int dC[4] = { 1, 0, -1, 0 };
	int runCnt = 0;
	bool color = false;

	while (cellsVisCount < cellsCount) {
		if (runCnt == 0) {
			runCnt = compressedData[dataIdx++];
			color = !color;
			continue;
		}

		img.at<uchar>(i, j) = (color ? dominantColor : blockColor);
		++cellsVisCount;
		--runCnt;

		int toR = i + dR[dir];
		int toC = j + dC[dir];

		if (toR > down) {
			--right;
			dir = (dir == 3) ? 0 : dir + 1;
		}
		else if (toR < up) {
			++left;
			dir = (dir == 3) ? 0 : dir + 1;
		}
		else if (toC > right) {
			++up;
			dir = (dir == 3) ? 0 : dir + 1;
		}
		else if (toC < left) {
			--down;
			dir = (dir == 3) ? 0 : dir + 1;
		}

		i += dR[dir];
		j += dC[dir];
	}
}

void Compressor::decodeRunLengthZigZag(cv::Mat& img) {
	// Retrieve image rows & cols count
	int rows = compressedData[dataIdx++];
	int cols = compressedData[dataIdx++];

	// Retrieve image pixels
	img = cv::Mat(rows, cols, CV_8U);

	int i = img.rows - 1, j = img.cols - 1;
	int cellsVisCount = 0, cellsCount = img.rows * img.cols;
	int dir = 0;
	int dR[2] = { 1, -1 };
	int dC[2] = { -1, 1 };
	int runCnt = 0;
	bool color = false;

	while (cellsVisCount < cellsCount) {
		if (runCnt == 0) {
			runCnt = compressedData[dataIdx++];
			color = !color;
			continue;
		}

		img.at<uchar>(i, j) = (color ? dominantColor : blockColor);
		++cellsVisCount;
		--runCnt;

		i += dR[dir];
		j += dC[dir];

		if (i < 0) {
			i = 0;
			j -= 2;
			dir = 1 - dir;
		}
		else if (j < 0) {
			j = 0;
			i -= 2;
			dir = 1 - dir;
		}
		else if (i >= img.rows) {
			i = img.rows - 1;
			dir = 1 - dir;
		}
		else if (j >= img.cols) {
			j = img.cols - 1;
			dir = 1 - dir;
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