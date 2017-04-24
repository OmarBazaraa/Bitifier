#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
using namespace std;

/**
 * Returns a list of files (name, extension) in the given directory
 */
void GetFilesInDirectory(vector<pair<string, string>>& files, const string& directory) {
	// Make sure to change the following command to the corresponding
	// one on your operating system when using Linux or MAC
	string s = "dir " + directory + "b > dirs.txt";
	system(s.c_str());

	ifstream fin("dirs.txt");

	while (getline(fin, s)) {
		int idx = (int)s.size() - 1;
		while (idx >= 0 && s[idx] != '.') --idx;
		files.push_back({ s.substr(0, idx), s.substr(idx + 1) });
	}

	remove("dirs.txt");
}