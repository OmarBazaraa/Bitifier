#include "Huffman.h"

//
// Encoding functions
//

void Huffman::encode(const vector<uchar>& data, vector<uchar>& encodedData) {
	buildCodeTable(data);
	encodeCodeTable(encodedData);

	uchar byte = 0;
	int bitsCount = 8;

	for (int i = 0; i < data.size(); ++i) {
		string& code = codeTable[data[i]];

		for (int j = 0; j < code.size(); ++j) {
			byte |= (code[j] - '0') << --bitsCount;

			if (bitsCount <= 0) {
				encodedData.push_back(byte);
				byte = 0;
				bitsCount = 8;
			}
		}
	}

	if (bitsCount < 8) {
		encodedData.push_back(byte);
		encodedData.push_back(bitsCount);
	}
	else {
		encodedData.push_back(0);
	}
}

void Huffman::encodeCodeTable(vector<uchar>& encodedData) {
	// Encode number of distinct symbols
	encodedData.push_back(codeTable.size() - 1);

	// Loop through each symbol to encode the symbol and its code word length
	for (auto it : codeTable) {
		encodedData.push_back(it.first);
		encodedData.push_back(it.second.size());
	}

	uchar byte = 0;
	int bitsCount = 8;

	// Loop through each symbol to encode its code word
	for (auto it : codeTable) {
		string& code = it.second;

		for (int j = 0; j < code.size(); ++j) {
			byte |= (code[j] - '0') << --bitsCount;

			if (bitsCount <= 0) {
				encodedData.push_back(byte);
				byte = 0;
				bitsCount = 8;
			}
		}
	}

	if (bitsCount < 8) {
		encodedData.push_back(byte);
	}
}

void Huffman::buildCodeTable(const vector<uchar>& data) {
	// Count the frequency of each symbol in the given data
	map<uchar, int> frq;
	for (int i = 0; i < data.size(); ++i) {
		++frq[data[i]];
	}

	// Populate symbols multiset sorted in non-decreasing order
	multiset<pair<int, Node*>> symbols;
	for (auto it : frq) {
		Node* n = new Node();
		n->symbol = it.first;
		symbols.insert({ it.second, n });
	}

	// Build the tree
	while (symbols.size() > 1) {
		int freq = 0;
		Node* n = new Node();
		
		// Combine the least frequent two nodes into one node

		// First node
		freq += symbols.begin()->first;
		n->left = symbols.begin()->second;
		symbols.erase(symbols.begin());
		// Second node
		freq += symbols.begin()->first;
		n->right = symbols.begin()->second;
		symbols.erase(symbols.begin());

		symbols.insert({ freq, n });
	}

	// Traverse the tree to generate the code table
	if (symbols.size() == 1) {
		Node* root = symbols.begin()->second;
		traverseTree(root);
		deleteTree(root);
	}
}

void Huffman::traverseTree(Node* node, string code) {
	if (node->left != NULL) {
		traverseTree(node->left, code + "0");
	}
	if (node->right != NULL) {
		traverseTree(node->right, code + "1");
	}

	// If the current node is a leaf node then insert its code into the table
	if (node->left == NULL && node->left == NULL) {
		codeTable.insert({ node->symbol, code });
	}
}

//
// Decoding functions
//

void Huffman::decode(const vector<uchar>& data, vector<uchar>& decodedData) {
	decodeCodeTable(data);

	// Convert data to binary string
	string binaryStr;
	while (dataIdx + 2 < data.size()) {
		binaryStr += byteToBinaryString(data[++dataIdx]);
	}

	int ingnoreCount = data.back();
	binaryStr.erase(binaryStr.size() - ingnoreCount, binaryStr.size());

	//
	string code;
	for (int i = 0; i < binaryStr.size(); ++i) {
		code += binaryStr[i];

		if (codeTableInverse.count(code)) {
			decodedData.push_back(codeTableInverse[code]);
			code.clear();
		}
	}
}

void Huffman::decodeCodeTable(const vector<uchar>& data) {
	dataIdx = -1;

	// Decode distinct symbols size
	int symbolsCount = data[++dataIdx] + 1;

	// Decode the symbols and their code word lengths
	int totalCodewordLengths = 0;
	vector<pair<uchar, int>> symbols(symbolsCount);
	for (int i = 0; i < symbolsCount; ++i) {
		symbols[i].first = data[++dataIdx];
		symbols[i].second = data[++dataIdx];
		totalCodewordLengths += symbols[i].second;
	}

	// 
	string codewords;
	int codewordsBytesCount = (totalCodewordLengths + 7) / 8;	// ceil(totalCodewordLengths / 8)
	for (int i = 0; i < codewordsBytesCount; ++i) {
		codewords += byteToBinaryString(data[++dataIdx]);
	}

	// 
	int l = 0;
	for (int i = 0; i < symbolsCount; ++i) {
		int len = symbols[i].second;
		codeTable[symbols[i].first] = codewords.substr(l, len);
		codeTableInverse[codewords.substr(l, len)] = symbols[i].first;
		l += len;
	}
}

//
// Helper functions
//

string Huffman::byteToBinaryString(uchar byte) {
	string result;
	for (int i = 7; i >= 0; --i) {
		result += '0' + (bool)(byte & (1 << i));
	}
	return result;
}

void Huffman::printCodeTable(string path) {
	ofstream fout(path);

	for (auto it : codeTable) {
		fout << (int)it.first << "\t" << it.second << endl;
	}

	fout.close();
}

void Huffman::deleteTree(Node* node) {
	if (node == NULL)
		return;

	deleteTree(node->left);
	deleteTree(node->right);
	delete node;
}