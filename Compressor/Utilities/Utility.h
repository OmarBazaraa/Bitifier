#pragma once
// STL libraries
#include <string>

// OpenCV libraries
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
using namespace std;

#define BLACK_WHITE_THRESHOLD	180

/**
 * Loads binary image from the given path into a matrix of pixels
 */
inline cv::Mat loadBinaryImage(const string& path) {
	// Load colored image from file
	cv::Mat grayMat = imread(path, CV_LOAD_IMAGE_GRAYSCALE);

	// Check for invalid input
	if (grayMat.empty() || !grayMat.data) {
		string errorMessage = "Could not load the image at: " + path;
		throw exception(errorMessage.c_str());
	}

	// Binary image
	cv::Mat binaryMat(grayMat.size(), grayMat.type());

	// Apply thresholding
	cv::threshold(grayMat, binaryMat, BLACK_WHITE_THRESHOLD, 255, cv::THRESH_BINARY);
	
	return binaryMat;
}

/**
 * Save the given vector of bytes to the given directory
 */
inline void saveFile(const string& path, const vector<uchar>& outBytes) {
	ofstream fout(path, ofstream::binary);

	if (!fout.is_open()) {
		string errorMessage = "Could not load the file at: " + path;
		throw exception(errorMessage.c_str());
	}

	fout.write((char*)outBytes.data(), outBytes.size());

	fout.close();
}

/**
 * Load the data from the given directory into the given vector of bytes
 */
inline void loadFile(const string& path, vector<uchar>& inBytes) {
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
	inBytes.resize(fileSize);
	fin.read((char*)inBytes.data(), fileSize);

	fin.close();
}

/**
 * Compare the given two images and return true if they match,
 * false otherwise
 */
inline bool compareImages(const cv::Mat& img1, const cv::Mat& img2) {
	// treat two empty mat as identical
	if (img1.empty() && img2.empty())
		return true;

	// if dimensions of two mat is not identical, these two mat is not identical
	if (img1.cols != img2.cols || img1.rows != img2.rows || img1.dims != img2.dims)
		return false;

	// Compare every pixel
	for (int i = 0; i < img1.rows; ++i)
		for (int j = 0; j < img1.cols; ++j)
			if (img1.at<bool>(i, j) != img2.at<bool>(i, j))
				return false;

	return true;
}