#include <iostream>
#include <ctime>
#include <iomanip>
#include "Directory.h"
#include "Utility.h"
#include "Compressor.h"
using namespace std;

#define PATH_SAMPLE_DATA        "DataSet/"
#define PATH_COMPRESSED_DATA    "Compressed/"
#define PATH_UNCOMPRESSED_DATA  "Uncompressed/"
#define EXT_SAMPLE_FILE         "jpg"
#define EXT_COMPRESSED_FILE     "cpr"

/**
 * Used to boost reading/writing from/to the console
 */
inline void boostIO() {
	ios_base::sync_with_stdio(0);
	cin.tie(0);
	cout.tie(0);
}

/**
 * Main function
 */
int main() {
	boostIO();
	int startTime = clock();
	int originalFilesSize = 0;
	int compressedFilesSize = 0;
	vector<pair<string, string>> files;

	cout << fixed << setprecision(3);
	
	try {
		// Read files info
		getFilesInDirectory(PATH_SAMPLE_DATA, files);

		for (int i = 0; i < files.size(); ++i) {
			// If file is not of type .jpg then skip it
			if (files[i].second != EXT_SAMPLE_FILE) {
				continue;
			}

			// Get file pathes
			string src = PATH_SAMPLE_DATA + files[i].first + "." + EXT_SAMPLE_FILE;
			string cpr = PATH_COMPRESSED_DATA + files[i].first + "." + EXT_COMPRESSED_FILE;
			string dst = PATH_UNCOMPRESSED_DATA + files[i].first + "." + EXT_SAMPLE_FILE;

			// Compression variables
			Compressor compressor;
			cv::Mat originalImg, uncompressedImg;
			vector<uchar> compressedBytes;

			// Loading image
			cout << "Loading " << src << "..." << endl;
			originalImg = loadBinaryImage(src);
			int orgSize = originalImg.rows * originalImg.cols;
			originalFilesSize += orgSize;

			// Compressing
			cout << "Compressing..." << endl;
			compressor.compress(originalImg, compressedBytes);
			int comSize = compressedBytes.size();
			compressedFilesSize += comSize;

			// Saving compressed image
			cout << "Saving compressed file..." << endl;
			saveFile(cpr, compressedBytes);
			
			// Loading file
			cout << "Loading compressed file..." << endl;
			loadFile(cpr, compressedBytes);

			// Extracting
			cout << "Extracting..." << endl;
			compressor.extract(compressedBytes, uncompressedImg);

			// Saving extracted image
			cout << "Saving image..." << endl;
			imwrite(dst, uncompressedImg);

			// Stop if invalid compression is detected
			cout << "Comparing original and compressed images..." << endl;
			if (!compareImages(originalImg, uncompressedImg)) {
				cout << "Lossy compression!" << endl;
				return 0;
			}

			cout << "Compression ratio: " << (double)orgSize / comSize << endl;
			cout << "------------------------------------" << endl << endl;
		}
	}
	catch (const exception& ex) {
		cout << "ERROR::" << ex.what() << endl;
	}

	// Output average compression ratio
	cout << "Total compression ratio: " << (double) originalFilesSize / compressedFilesSize << endl << endl;

	// Output process time
	int stopTime = clock();
	cout << "Time: " << (stopTime - startTime) / double(CLOCKS_PER_SEC) << "sec" << endl;
	return 0;
}