#pragma once
// STL libraries
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>

// OpenCV libraries
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Custom libraries
#include "ByteConcatenator.h"
#include "Huffman.h"

using namespace cv;
using namespace std;

bool cmp(const pair<vector<int>, int>& lhs, const pair<vector<int>, int>& rhs);

class Compressor
{
private:
	// Image variables
	cv::Mat imageMat;
	uchar dominantColor = 255;
	uchar blockColor = 0;
	vector<cv::Mat> shapes;                 // Vector of distinct shapes matrices
	vector<vector<int>> shapeBlocks;        // Vector holding the block indecies for each distinct shape
	vector<pair<int, int>> imageBlocks;     // Vector holding all image blocks starting pixel and the reference shape index
	unordered_map<int, int> blockShapes;    // Maps block to its reference shape

	// Compressed data variables
	int dataIdx = 0;
	vector<int> compressedData;
	vector<uchar> concatenatedData;

	// DFS variables
	cv::Mat visited;
	int dirR[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
	int dirC[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	int minRow, minCol, maxRow, maxCol;

	// Run-Length encoding types
	const int RUN_LENGTH_HOR = 0;
	const int RUN_LENGTH_VER = 1;
	const int RUN_LENGTH_SPIRAL = 2;
	const int RUN_LENGTH_ZIGZAG = 3;

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
	 * Encode image distinct shapes after detecting them by calling detectImageBlocks function
	 */
	void encodeDistinctShapes();

	/**
	 *
	 */
	void applySymmetry(cv::Mat& img);

	/**
	 * Encode image blocks upper left pixel indecies
	 */
	void encodeImageBlocks();

	/**
	 * Encode the given image using run length encoding algorithm in horizontal mannar
	 */
	void encodeRunLengthHorizontal(const cv::Mat& img, vector<int>& encodedData);

	/**
	 * Encode the given image using run length encoding algorithm in vertical mannar
	 */
	void encodeRunLengthVertical(const cv::Mat& img, vector<int>& encodedData);

	/**
	 * Encode the given image using run length encoding algorithm in spiral mannar
	 */
	void encodeRunLengthSpiral(const cv::Mat& img, vector<int>& encodedData);

	/**
	 * Encode the given image using run length encoding algorithm in zig-zag mannar
	 */
	void encodeRunLengthZigZag(const cv::Mat& img, vector<int>& encodedData);

	/**
	 * Encode meta-data needed in decompression process
	 */
	void encodeMetaData();

	// ==============================================================================
	//
	// Compression helper functions
	//
private:
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
	 * Detect the dominat color of the image
	 */
	void detectDominantColor();

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
	 * Decode image distinct shapes and their refering blocks indecies
	 */
	void decodeDistinctShapes();

	/**
	 * Decode image blocks starting pixel indecies
	 */
	void decodeImageBlocks();

	/**
	 * Decode the given encoded image using run length decoding algorithm in horizontal mannar
	 */
	void decodeRunLengthHorizontal(cv::Mat& img);

	/**
	 * Decode the given encoded image using run length decoding algorithm in vertical mannar
	 */
	void decodeRunLengthVertical(cv::Mat& img);

	/**
	 * Decode the given encoded image using run length decoding algorithm in spiral mannar
	 */
	void decodeRunLengthSpiral(cv::Mat& img);

	/**
	 * Decode the given encoded image using run length decoding algorithm in zig-zag mannar
	 */
	void decodeRunLengthZigZag(cv::Mat& img);

	/**
	 * Decode image compressed meta-data needed in decompression process
	 */
	void decodeMetaData();
};