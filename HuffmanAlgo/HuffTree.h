#ifndef HUFFTREE_H
#define HUFFTREE_H
#include "Bitset.h"
#include "Heap.h"

using ull = unsigned long long;
using uchar = unsigned char;

class HuffTree {
public:
	struct HuffNode {
		HuffNode(HuffNode *mLeft, HuffNode *mRight, int mWeight, char mC, bool mLeaf)
			: left(mLeft), right(mRight), weight(mWeight), c(mC), leaf(mLeaf) {}
		HuffNode *left;
		HuffNode *right;
		int weight;
		unsigned char c;
		bool leaf;
	};
	HuffTree();
	~HuffTree();
    HuffTree(const HuffTree &other) = delete;
	HuffTree &operator=(const HuffTree &other) = delete;

protected:
	HuffNode *root;
	void delHTree(HuffNode *node);
};

class CompressTree : public HuffTree {
public:
	CompressTree(const std::string &str);
	void write(std::ostream &out) const;
	std::vector<Bitset<ull>*> buildTable() const;
	Bitset<ull> compress(const std::string &str) const;
private:
	int uniqueChars;
	static const int alphabetSize = 256;
	int charCount[alphabetSize];
	struct Comp {
		bool operator()(const HuffNode *lhs, const HuffNode *rhs) const {
			return lhs->weight < rhs->weight;
		}
	};
	void buildHTree(const std::string &str);
	void buildBitsetFromTree(const HuffNode *node, Bitset<uchar> &tree, std::string& chars) const;
	void buildTable(const HuffTree::HuffNode* node, Bitset<ull>& curr, std::vector<Bitset<ull>*>& codeTable) const;

};

class DecompressTree : public HuffTree {
public:
	DecompressTree(const Bitset<uchar> &tree, const std::string &chars);
	std::string decompress(const Bitset<ull> &code) const;
private:
	HuffNode* buildTreeFromBitset(const Bitset<uchar> &tree, const std::string &chars, int &idx, int &charIdx);
	std::string decompress(HuffTree::HuffNode *root, const Bitset<ull> &code) const;
};
#endif