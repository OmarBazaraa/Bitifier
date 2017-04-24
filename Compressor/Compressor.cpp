#include "Compressor.h"

Compressor::Compressor(int threshold) {
	this->threshold = threshold;
}

Compressor::~Compressor() {
	imgMatrix.deallocate();
}

//
// Compression functions
//

void Compressor::compress(const string& imagePath, const string& outputPath) {
	cout << "Loading image..." << endl;
	loadImage(imagePath);
	cout << "Compressing..." << endl;
	encodeData();
	cout << "Saving compressed file..." << endl;
	saveCompressedFile(outputPath);
}

void Compressor::loadImage(const string& path) {
	// Load colored image from file
	cv::Mat rgbMat = imread(path, CV_LOAD_IMAGE_COLOR);

	// Check for invalid input
	if (rgbMat.empty() || !rgbMat.data) {
		string errorMessage = "Could not load the image at: " + path;
		throw exception(errorMessage.c_str());
	}

	// Get image size
	rows = rgbMat.rows;
	cols = rgbMat.cols;

	// Convert BGR to Gray
	cv::cvtColor(rgbMat, imgMatrix, CV_BGR2GRAY);
}

void Compressor::encodeData() {
	compressedBytes.clear();
	compressedSizes.clear();

	// Store image rows count
	compressedSizes.push_back(encodeToBase256(rows));

	// Store image cols count
	compressedSizes.push_back(encodeToBase256(cols));

	// Store image pixels
	int cnt = 0;
	bool pixel, prv = true;
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			pixel = ((int)imgMatrix.at<uchar>(i, j) > this->threshold);

			if (prv == pixel) {
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
	/*for (int i = 0; i < compressedSizes.size(); ++i) {
		compressedImage.push_back(compressedSizes[i]);
	}*/
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

void Compressor::saveCompressedFile(const string& path) {
	ofstream fout(path, ofstream::binary);

	if (!fout.is_open()) {
		string errorMessage = "Could not load the file at: " + path;
		throw exception(errorMessage.c_str());
	}
	
	fout.write((char*)compressedBytes.data(), compressedBytes.size());

	fout.close();
}

//
// Extraction functions
//

void Compressor::extract(const string& compressedFilePath, const string& outputPath) {
	cout << "Loading compressed file..." << endl;
	loadCompressedFile(compressedFilePath);
	cout << "Extracting..." << endl;
	decodeData();
	cout << "Saving image..." << endl;
	saveImage(outputPath);
}

void Compressor::loadCompressedFile(const string& path) {
	ifstream fin(path, ifstream::binary);
	
	if (!fin.is_open()) {
		string errorMessage = "Could not load the file at: " + path;
		throw exception(errorMessage.c_str());
	}

	// Get file size
	fin.seekg(0, fin.end);
	int fileSize = fin.tellg();
	fin.seekg(0, fin.beg);

	// Read all data
	compressedBytes.resize(fileSize);
	fin.read((char*)compressedBytes.data(), fileSize);

	fin.close();
}

void Compressor::decodeData() {
	// Retrieve image meta-data
	compressedSizes.clear();
	int bytesCnt = 0;
	int dataIdx = 0, sizeIdx = -1, idx = -1;
	int n = compressedBytes.size();
	while (n >= 0 && n > bytesCnt) {
		int cnt = (compressedBytes[--n] & 63);
		int len = (compressedBytes[n] >> 6) + 1;
		bytesCnt += cnt * len;

		for (int i = 0; i < cnt; ++i) {
			compressedSizes.push_back(len);
		}
	}

	/*compressedSizes.clear();
	int bytesCnt = 0;
	int dataIdx = 0, sizeIdx = -1;
	int n = compressedImage.size();
	while (n >= 0 && n > bytesCnt) {
		bytesCnt += (int)compressedImage[--n];
		compressedSizes.push_back(compressedImage[n]);
	}
	reverse(compressedSizes.begin(), compressedSizes.end());*/

	if (n != bytesCnt) {
		throw exception("Could not extract the given file");
	}

	// Retrieve image rows count
	rows = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];

	// Retrieve image cols count
	cols = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
	dataIdx += compressedSizes[sizeIdx];

	// Retrieve image pixels
	imgMatrix = cv::Mat(rows, cols, CV_8U);
	int i = 0, j = 0;
	int cnt;
	bool color = true;
	while (dataIdx < n) {
		cnt = decodeFromBase256(dataIdx, compressedSizes[++sizeIdx]);
		dataIdx += compressedSizes[sizeIdx];

		while (cnt--) {
			imgMatrix.at<uchar>(i, j) = color ? 255 : 0;

			if (++j >= cols) {
				++i;
				j = 0;
			}
		}

		color = !color;
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

void Compressor::saveImage(const string& path) {
	//Mat image(rows, cols, CV_8UC3);

	/*int idx = -1;
	for (int i = 0; i < rows; ++i)
	for (int j = 0; j < cols; ++j)
	image.at<Vec3b>(i, j) = (binaryImage[++idx] ? Vec3b(255, 255, 255) : Vec3b(0, 0, 0));*/

	imwrite(path, imgMatrix);
}