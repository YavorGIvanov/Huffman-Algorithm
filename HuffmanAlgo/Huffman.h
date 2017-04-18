#include "HuffTree.h"

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
	const CompressTree compressTree(str);

	/// 3. Open/Create the output binary file
	std::ofstream out(outFile, std::ios::out | std::ios::binary);
	if (!out) {
		return false;
	}

	/// 4. Write the header which can be used decode the file
	compressTree.write(out);

	/// 5. Compute the result of the string compression in a bitset
	Bitset<ull> res = compressTree.compress(str);

	/// 6. Write the bitset to the file
	res.write(out);

	return true;
}

bool decompressFile(const char *inFile, const char *outFile) {
	/// 0. Open the decompressed input file
	std::ifstream in(inFile, std::ios::binary);
	if (!in) {
		return false;
	}

	/// 1. Read the header to create a Huffman Tree for decompression
	Bitset<uchar> tree(in);
	int sz;
	in.read(reinterpret_cast<char *>(&sz), sizeof(sz));
	std::string chars(sz, ' ');
	in.read(&chars[0], sz);
	const DecompressTree decompressTree(tree, chars);
	/// 2. Read the compressed code in a Bitset
	Bitset<ull> code(in);
	in.close();
	/// 3. Decompress the bitset code in a char buffer
	std::string str = decompressTree.decompress(code);
	/// 4. Write the char buffer to the decompressed file
	std::ofstream out(outFile, std::ios::binary);
	if (!out) {
		return false;
	}
	out.write(reinterpret_cast<char *>(&str[0]), str.size());
	out.close();
}