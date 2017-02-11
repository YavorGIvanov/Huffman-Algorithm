#ifndef HUFFMAN_H
#define HUFFMAN_H
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include "HuffTree.h"
#include <vector>
typedef unsigned long long ull;
typedef unsigned char uchar;
typedef std::vector<Bitset<ull>*> vecBitsetPtrs;

void buildTable(const HuffTree::HuffNode* node, Bitset<ull>& curr, vecBitsetPtrs& codeTable) {
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
	FILE *in = fopen(inFile, "rb");
	if (!in) {
		perror("Error with opening input file\n");
		return false;
	}
	/// 1. Read the input file in a char buffer
	fseek(in, 0, SEEK_END);
	int sz = ftell(in);
	fseek(in, 0, SEEK_SET);
	CharBuffer str(in, sz);
	fclose(in);

	/// 2. Create a Huffman Tree from the given string
	const HuffTree compressTree(str);

	/// 3. Open/Create the output binary file
	FILE *out = fopen(outFile, "wb+");
	if (!out) {
		perror("Error with opening/creating output file\n");
		return false;
	}

	/// 4. Write the header which can be used decode the file
	compressTree.writeTreeToFile(out);
	
	/// 5. Compute the result of the string compression in a bitset
	HuffTree::HuffNode *root = compressTree.getRoot();
	vecBitsetPtrs codeTable(256);
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
	res.writeToFile(out);
	fclose(out);
	/// 7. Delete pointers in codeTable
	for (int i = 0; i < codeTable.size(); i++) {
		delete codeTable[i];
	}
	return true;
}


CharBuffer decompress(HuffTree::HuffNode *root, const Bitset<ull> &code) {
	CharBuffer res(BUFSIZ);
	HuffTree::HuffNode *node;
	int i = 0;
	while (i < code.size()) {
		node = root;
		while (!node->leaf) {
			node = code[i++] ? node->right : node->left;
		}
		res.push_back(node->c);
	}
	return res;
}

bool decompressFile(const char *inFile, const char *outFile) {
	/// 0. Open the decompressed input file
	FILE *in = fopen(inFile, "rb");
	if (!in) {
		perror("Error with opening the input compressed file\n");
		return false;
	}

	/// 1. Read the header to create a Huffman Tree for decompression
	Bitset<uchar> tree(in);
	int sz;
	fread(&sz, sizeof(sz), 1, in);
	CharBuffer chars(in, sz);
	HuffTree decompressTree(tree, chars);
	HuffTree::HuffNode *root = decompressTree.getRoot();
	///2. Read the compressed code in a Bitset
	Bitset<ull> code(in);
	fclose(in);
	/// 3. Decompress the bitset code in a char buffer
	CharBuffer str = decompress(root, code);
	/// 4. Write the char buffer to the decompressed file
	FILE *out = fopen(outFile, "wb");
	if (!out) {
		perror("Error with opening the output for decompress file\n");
		return false;
	}
	str.writeToFileNoSz(out);

	fclose(out);
}
#endif