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

/**
 * Loads binary image from the given path into a matrix of pixels
 */
cv::Mat loadBinaryImage(const string& path, int threshold) {
	// Load colored image from file
	cv::Mat rgbMat = imread(path, CV_LOAD_IMAGE_COLOR);

	// Check for invalid input
	if (rgbMat.empty() || !rgbMat.data) {
		string errorMessage = "Could not load the image at: " + path;
		throw exception(errorMessage.c_str());
	}

	// Grayscale matrix
	cv::Mat grayMat(rgbMat.size(), CV_8U);

	// Convert BGR to Gray
	cv::cvtColor(rgbMat, grayMat, CV_BGR2GRAY);

	// Binary image
	cv::Mat binaryMat(grayMat.size(), grayMat.type());

	// Apply thresholding
	cv::threshold(grayMat, binaryMat, threshold, 255, cv::THRESH_BINARY);

	return binaryMat;
}

/**
 * Compare the given two images and return true if they match,
 * false otherwise
 */
bool compareImages(const string& image1, const string& image2, int threshold) {
	// Load images
	Mat img1 = loadBinaryImage(image1, threshold);
	Mat img2 = loadBinaryImage(image2, threshold);

	// treat two empty mat as identical
	if (img1.empty() && img2.empty()) {
		return true;
	}

	// if dimensions of two mat is not identical, these two mat is not identical
	if (img1.cols != img2.cols || img1.rows != img2.rows || img1.dims != img2.dims) {
		return false;
	}

	// Compare every pixel
	for (int i = 0; i < img1.rows; ++i)
		for (int j = 0; j < img1.cols; ++j)
			if (img1.at<uchar>(i, j) != img1.at<uchar>(i, j))
				return false;

	return true;
}