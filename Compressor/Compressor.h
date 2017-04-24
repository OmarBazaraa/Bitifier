#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
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
	 * Encode the given black & white image
	 */
	void encode(const string& imagePath, const string& outputPath);

	/**
	 * Decode the compressed file back to a black & white image
	 */
	void decode(const string& compressedFilePath, const string& outputPath);

private:
	/**
	 * Loads grayscaled image from the given path into a matrix
	 */
	void loadImage(const string& path);

	/**
	 * Compress the loaded image and save it to compressedBytes vector
	 */
	void compress();

	/**
	 * Compress meta-data needed in decompression process
	 */
	void compressMetaData();

	/**
	 * Save the compress data to the given path
	 */
	void saveCompressedFile(const string& path);

	/**
	 * Load the compress data from the given path
	 */
	void loadCompressedFile(const string& path);

	/**
	 * Decompress the loaded file into a binary image
	 */
	void decompress();

	/**
	 * Save the loaded image with the given url
	 */
	void saveImage(const string& path);

private:
	/**
	 * Convert the given integer to base 256 and stack them on
	 * compressedBytes vector.
	 * Return the number of digits of the given number in base 256
	 */
	int saveInBase256(int number);

	/**
	 * Convert the given size of bytes from compressedBytes vector starting from idx
	 * from base 256 to decimal.
	 * Return the converted integer
	 */
	int loadFromBase256(int idx, int size);
};