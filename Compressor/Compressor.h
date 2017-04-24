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
//#include "Utility.h"
using namespace cv;
using namespace std;

class Compressor
{
public:
	int rows;
	int cols;
	int threshold;
	vector<int> compressedSizes;
	vector<unsigned char> compressedBytes;
	cv::Mat imgMatrix;

public:
	/**
	 * Constructor
	 */
	Compressor(int threshold);

	/**
	 * Destructor
	 */
	~Compressor();

	/**
	 * Compress the given black & white jpg image
	 */
	void compress(const string& imagePath, const string& outputPath);

	/**
	 * Extract the given compressed file to a black & white jpg image
	 */
	void extract(const string& compressedFilePath, const string& outputPath);

private:
	/**
	 * Loads grayscaled image from the given path into a matrix
	 */
	void loadImage(const string& path);

	/**
	 * Encode the loaded image data and save it to compressedBytes vector
	 */
	void encodeData();

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
	 * Save the compress data to the given path
	 */
	void saveCompressedFile(const string& path);

	/**
	 * Load the compress data from the given path
	 */
	void loadCompressedFile(const string& path);

	/**
	 * Decode the loaded file data into a binary image
	 */
	void decodeData();

	/**
	 * Convert the given size of bytes from compressedBytes vector starting from idx
	 * from base 256 to decimal.
	 * Return the converted integer
	 */
	int decodeFromBase256(int idx, int size);

	/**
	 * Save the loaded image with the given url
	 */
	void saveImage(const string& path);	
};