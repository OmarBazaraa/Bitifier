#pragma once
// STL libraries
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

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
	vector<int> compressedSizes;
	vector<unsigned char> compressedBytes;

public:
	/**
	 * Constructor
	 */
	Compressor();

	/**
	 * Destructor
	 */
	~Compressor();

	/**
	 * Compress the given black & white jpg image
	 */
	void compress(const cv::Mat& imageMat, vector<uchar>& outputBytes);

	/**
	 * Extract the given compressed file to a black & white jpg image
	 */
	void extract(vector<uchar>& compressedBytes, cv::Mat& outputImage);

private:
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