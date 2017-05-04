#pragma once
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <set>
using namespace std;

typedef unsigned char uchar;

class Huffman
{
	/**
	 * Node structure needed to build Huffman tree
	 */
	struct Node {
		uchar symbol;
		Node* left = NULL;
		Node* right = NULL;
		int frequency = 0;
	};

private:
	map<uchar, string> codeTable;

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
	void encodeCodeTable(vector<uchar>& encodedData);

	void buildCodeTable(const vector<uchar>& data);

	void traverseTree(Node* node, string code = "");

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
	void decodeCodeTable(const vector<uchar>& data);

	//
	// Helper functions
	//
private:
	void deleteTree(Node* node);
};