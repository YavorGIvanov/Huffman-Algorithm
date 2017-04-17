#ifndef HUFFMAN_H
#define HUFFMAN_H
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include "HuffTree.h"
#include <vector>
#include <string>
#include <fstream>

using ull = unsigned long long;
using uchar = unsigned char;

void buildTable(const HuffTree::HuffNode* node, Bitset<ull>& curr, std::vector<Bitset<ull>*>& codeTable) {
	if (node->leaf) {
		codeTable[node->c] = new Bitset<ull>(curr);
		return;
	}
	curr.push_back(0);
	buildTable(node->left, curr, codeTable);
	curr.set(curr.size() - 1);
	buildTable(node->right, curr, codeTable);
	curr.pop_back();
}

bool compressFile(const char *inFile, const char *outFile) {
	/// 0. Open the input file
	std::ifstream in(inFile, std::ios::in | std::ios::binary);
	if (!in) {
		return false;
	}
	/// 1. Read the input file in a char buffer
	in.seekg(0, std::ios::end);
	std::string str(in.tellg(), ' ');
	in.seekg(0, std::ios::beg);
	in.read(&str[0], str.size());
	in.close();

	/// 2. Create a Huffman Tree from the given string
	const CompressHuffTree compressTree(str);

	/// 3. Open/Create the output binary file
	std::ofstream out(outFile, std::ios::out | std::ios::binary);
	if (!out) {
		return false;
	}

	/// 4. Write the header which can be used decode the file
	compressTree.write(out);
	
	/// 5. Compute the result of the string compression in a bitset
	HuffTree::HuffNode *root = compressTree.getRoot();
	std::vector<Bitset<ull>*> codeTable(256);
	buildTable(root, Bitset<ull>(), codeTable);

	///6. Calculate size in bits of the output compressed code
	int codeBitSize = 0;
	const int *charCount = compressTree.getCharCountArr();
	for (int i = 0; i < 256; i++) {
		if (codeTable[i]) {
			codeBitSize += charCount[i] * codeTable[i]->size();
		}
	}
	Bitset<ull> res(codeBitSize);

	for (int i = 0; i < str.size(); i++) {
		int k = str[i];
		res += *(codeTable[k]);
	};

	/// 6. Write the bitset to the file
	res.write(out);
	/// 7. Delete pointers in codeTable
	for (int i = 0; i < codeTable.size(); i++) {
		delete codeTable[i];
	}
	return true;
}


std::string decompress(HuffTree::HuffNode *root, const Bitset<ull> &code) {
	std::string res;
	res.reserve(BUFSIZ);
	HuffTree::HuffNode *node;
	int i = 0;
	while (i < code.size()) {
		node = root;
		while (!node->leaf) {
			node = code[i++] ? node->right : node->left;
		}
		res += node->c;
	}
	return res;
}

bool decompressFile(const char *inFile, const char *outFile) {
	/// 0. Open the decompressed input file
	std::ifstream in(inFile, std::ios::binary);
	if (!in) {
		perror("Error with opening the input compressed file\n");
		return false;
	}

	/// 1. Read the header to create a Huffman Tree for decompression
	Bitset<uchar> tree(in);
	int sz;
	in.read(reinterpret_cast<char *>(&sz), sizeof(sz));
	std::string chars(sz, ' ');
	in.read(&chars[0], sz);
	DecompressHuffTree decompressTree(tree, chars);
	///2. Read the compressed code in a Bitset
	Bitset<ull> code(in);
	in.close();
	/// 3. Decompress the bitset code in a char buffer
	std::string str = decompress(decompressTree.getRoot(), code);
	/// 4. Write the char buffer to the decompressed file
	std::ofstream out(outFile, std::ios::binary);
	if (!out) {
		return false;
	}
	out.write(reinterpret_cast<char*>(&str[0]), str.size());
	out.close();
}
#endif