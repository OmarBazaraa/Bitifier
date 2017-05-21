#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "BitConcatenator.h"
using namespace std;

typedef unsigned char uchar;

/**
 * Node structure needed to build Huffman tree
 */
struct SymbolNode {
	uchar symbol;
	SymbolNode* left = NULL;
	SymbolNode* right = NULL;
};

class Huffman
{
private:
	const int ALPHA_SIZE = 256;

	int dataIdx;
	string encodedData;
	vector<int> symbolsFrq;
	map<uchar, string> codeTable;
	map<string, uchar> codeTableInverse;

	// ==============================================================================
	//
	// Encoding functions
	//
public:	
	/**
	 * Encode the passed data by generating shorter code words for
	 * more frequent symbols
	 */
	void encode(const vector<uchar>& data, vector<uchar>& encodedData);

private:
	// [deprecated, encodeSymbols is used instead]
	void encodeCodeTable(vector<uchar>& encodedData);

	void encodeSymbols(vector<uchar>& encodedData);

	// ==============================================================================
	//
	// Decoding functions
	//
public:
	/**
	 * Decode the passed data by retrieving the code word table and mapping each code
	 * to its corresponding symbol
	 */
	void decode(const vector<uchar>& data, vector<uchar>& decodedData);

private:
	// [deprecated, decodeSymbols is used instead]
	void decodeCodeTable(const vector<uchar>& data);

	void decodeSymbols(const vector<uchar>& data);

	//
	// Helper functions
	//
private:
	void buildCodeTable();

	void traverseTree(SymbolNode* node, string code = "");

	string byteToBinaryString(uchar byte);

	void printCodeTable(string path);

	void deleteTree(SymbolNode* node);
};