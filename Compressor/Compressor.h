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
#include "Huffman.h"

using namespace cv;
using namespace std;

class Compressor
{
private:
	// Image variables
	cv::Mat imageMat;
	uchar dominantColor = 255;
	uchar blockColor = 0;
	vector<cv::Mat> shapes;
	vector<pair<int, int>> imageBlocks;

	// Compressed data variables
	int bytesIdx = 0;
	int sizesIdx = 0;
	vector<int> compressedSizes;
	vector<uchar> compressedBytes;

	// DFS variables
	cv::Mat visited;
	int dirR[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
	int dirC[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	int minRow, minCol, maxRow, maxCol;

	// ==============================================================================
	//
	// Compression functions
	//
public:
	/**
	 * Compress the given black & white jpg image
	 */
	void compress(const cv::Mat& imageMat, vector<uchar>& outputBytes);

private:
	/**
	 * Encode the image by detecting the repeated shapes and encode them once
	 */
	void encodeAdvanced();

	/**
	 * Detect the dominat color of the image
	 */
	void detectDominantColor();

	/**
	 * Detect image distinct shapes and map each image block to one of these shapes
	 */
	void detectImageBlocks();

	/**
	 * Store the given shape and return a unique number representing it
	 * if the shape already stored then it will not be inserted
	 */
	int storeUniqueShape(const cv::Mat& shape);

	/**
	 * Search the image using depth first search (DFS) algorithm to
	 * detect the boundaries of the sphape around the given point
	 */
	void dfs(int row, int col);

	/**
	 * Check whether the given point is valid in the DFS movement
	 */
	bool valid(int row, int col);

	/**
	 * Encode the given image using run length encoding algorithm
	 */
	void encodeRunLength(const cv::Mat& img);

	/**
	 * Encode meta-data needed in decompression process
	 */
	void encodeMetaData();

	/**
	 * Convert the given integer to base 256 and stack it and its size on
	 * compressedBytes and comressedSizes vectors respectivly
	 */
	void encodeToBase256(int number);

	// ==============================================================================
	//
	// Extraction functions
	//
public:
	/**
	* Extract the given compressed file to a black & white jpg image
	*/
	void extract(vector<uchar>& compressedBytes, cv::Mat& outputImage);

private:
	/**
	 * Decode the data by retrieving the distinct shapes then mapping all image blocks
	 * to one of the shapes
	 */
	void decodeAdvanced();

	/**
	 * Decode the given encoded image using run length decoding algorithm
	 */
	void decodeRunLength(cv::Mat& img);

	/**
	 * 
	 */
	void decodeImageBlocks();

	/**
	 * Decode image compressed meta-data needed in decompression process
	 */
	void decodeMetaData();

	/**
	 * Convert the currently pointed data from base 256 to decimal and increment the pointers.
	 * Return the converted integer
	 */
	int decodeFromBase256();
};