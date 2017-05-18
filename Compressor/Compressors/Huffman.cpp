#include "Huffman.h"

//
// Encoding functions
//

void Huffman::encode(const vector<uchar>& data, vector<uchar>& encodedData) {
	if (data.empty())
		return;

	// Count the frequency of each symbol in the given data
	symbolsFrq.resize(ALPHA_SIZE);
	for (int i = 0; i < data.size(); ++i) {
		++symbolsFrq[data[i]];
	}

	buildCodeTable();
	//encodeCodeTable(encodedData);
	encodeSymbols(encodedData);

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
		encodedData.push_back(bitsCount);	// Number of bits to be ignored
	}
	else {
		encodedData.push_back(0);			// Number of bits to be ignored
	}
}

// [deprecated, encodeSymbols is used instead]
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

void Huffman::encodeSymbols(vector<uchar>& encodedData) {
	ByteConcatenator concat;
	vector<uchar> metaData;
	concat.concatenate(symbolsFrq, metaData);

	int n = metaData.size();

	if (n >= 1 << 16) {
		throw exception("Cannot encode Huffman meta-data");
	}

	encodedData.push_back(n);
	encodedData.push_back(n >> 8);
	encodedData.insert(encodedData.end(), metaData.begin(), metaData.end());
}

// ==============================================================================
//
// Decoding functions
//

void Huffman::decode(const vector<uchar>& data, vector<uchar>& decodedData) {
	decodeSymbols(data);
	//decodeCodeTable(data);
	buildCodeTable();

	// Convert data to binary string
	string binaryStr;
	while (dataIdx + 2 < data.size()) {
		binaryStr += byteToBinaryString(data[++dataIdx]);
	}
	int ingnoreCount = data.back();
	binaryStr.erase(binaryStr.size() - ingnoreCount, binaryStr.size());

	// Detect code words and map them to their corresponding symbol
	string code;
	for (int i = 0; i < binaryStr.size(); ++i) {
		code += binaryStr[i];

		if (codeTableInverse.count(code)) {
			decodedData.push_back(codeTableInverse[code]);
			code.clear();
		}
	}
}

// [deprecated, decodeSymbols is used instead]
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

	// Loop to get all distinct code words concatenated together
	string codewords;
	int codewordsBytesCount = (totalCodewordLengths + 7) / 8;	// ceil(totalCodewordLengths / 8)
	for (int i = 0; i < codewordsBytesCount; ++i) {
		codewords += byteToBinaryString(data[++dataIdx]);
	}

	// Build Huffman code word table by spliting the previous string accourding to the lengths
	// of the code words
	int l = 0;
	for (int i = 0; i < symbolsCount; ++i) {
		int len = symbols[i].second;
		codeTable[symbols[i].first] = codewords.substr(l, len);
		codeTableInverse[codewords.substr(l, len)] = symbols[i].first;
		l += len;
	}
}

void Huffman::decodeSymbols(const vector<uchar>& data) {
	int n = 2;
	n += data[0];
	n += data[1] << 8;

	dataIdx = n - 1;

	ByteConcatenator concat;
	vector<uchar> metaData(data.begin() + 2, data.begin() + n);
	concat.deconcatenate(metaData, symbolsFrq);
}

// ==============================================================================
//
// Helper functions
//

void Huffman::buildCodeTable() {
	// Populate symbols multiset sorted in non-decreasing order
	multiset<pair<int, SymbolNode*>> symbols;
	for (int i = 0; i < ALPHA_SIZE; ++i) {
		if (symbolsFrq[i] == 0) continue;
		SymbolNode* n = new SymbolNode();
		n->symbol = i;
		symbols.insert({ symbolsFrq[i], n });
	}

	// Build the tree
	while (symbols.size() > 1) {
		int freq = 0;
		SymbolNode* n = new SymbolNode();

		//
		// Combine the least frequent two nodes into one node
		//
		// First node
		freq += symbols.begin()->first;
		n->left = symbols.begin()->second;
		symbols.erase(symbols.begin());
		// Second node
		freq += symbols.begin()->first;
		n->right = symbols.begin()->second;
		symbols.erase(symbols.begin());

		// Insert the combined node
		symbols.insert({ freq, n });
	}

	// Traverse the tree to generate the code table
	if (symbols.size() == 1) {
		SymbolNode* root = symbols.begin()->second;
		traverseTree(root);
		deleteTree(root);
	}
}

void Huffman::traverseTree(SymbolNode* node, string code) {
	if (node->left != NULL) {
		traverseTree(node->left, code + "0");
	}
	if (node->right != NULL) {
		traverseTree(node->right, code + "1");
	}

	// If the current node is a leaf node then insert its code into the table
	if (node->left == NULL && node->left == NULL) {
		codeTable.insert({ node->symbol, code });
		codeTableInverse.insert({ code, node->symbol });
	}
}

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

void Huffman::deleteTree(SymbolNode* node) {
	if (node == NULL)
		return;

	deleteTree(node->left);
	deleteTree(node->right);
	delete node;
}

bool operator<(const pair<int, SymbolNode*>& lhs, const pair<int, SymbolNode*> rhs) {
	return lhs.first < rhs.first;
}