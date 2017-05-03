#pragma once
// STL libraries
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>

// OpenCV libraries
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Custom libraries
using namespace cv;
using namespace std;

class Compressor
{
private:
	cv::Mat imageMat;
	vector<cv::Mat> shapes;
	vector<pair<int, int>> imageBlocks;
	vector<int> compressedSizes;
	vector<unsigned char> compressedBytes;

private:
	// DFS variables
	cv::Mat vis;
	int dirR[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
	int dirC[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	int minRow, minCol, maxRow, maxCol;

public:
	/**
	 * Compress the given black & white jpg image
	 */
	void compress(const cv::Mat& imageMat, vector<uchar>& outputBytes);

	/**
	 * Extract the given compressed file to a black & white jpg image
	 */
	void extract(vector<uchar>& compressedBytes, cv::Mat& outputImage);

private:

	void encodeAdvanced();
	int getShapeIdx(const cv::Mat& shape);
	void dfs(int row, int col);
	bool valid(int row, int col);

	void encodeRunLength(const cv::Mat& img);

	/**
	 * Encode meta-data needed in decompression process
	 */
	void encodeMetaData();

	/**
	 * Convert the given integer to base 256 and stack them on
	 * compressedBytes vector.
	 * Return the number of digits of the given number in base 256
	 */
	int encodeToBase256(int number);

	// ==============================================================================

	void decodeAdvanced();

	void decodeRunLength(cv::Mat& img, int& dataIdx, int& sizeIdx);

	/**
	 * Decode image compressed meta-data needed in decompression process
	 */
	void decodeMetaData();

	/**
	 * Convert the given size of bytes from compressedBytes vector starting from idx
	 * from base 256 to decimal.
	 * Return the converted integer
	 */
	int decodeFromBase256(int idx, int size);
};