#ifndef HUFFTREE_H
#define HUFFTREE_H
#include <vector>
#include <memory>
#include "Bitset.h"
#include "Heap.h"

class HuffTree {
	typedef unsigned long long ull;
	typedef unsigned char uchar;
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
	HuffNode *getRoot() const {
		return root;
	}
	HuffTree() : root{} {}
    HuffTree(const HuffTree &other) = delete;
	HuffTree &operator=(const HuffTree &other) = delete;

protected:
	HuffNode *root;
};

class CompressHuffTree : public HuffTree {
	typedef unsigned char uchar;
public:
	CompressHuffTree(const std::string &str)  {
		charCounter = new int[alphabetSize]();
		for (int i = 0; i < str.size(); ++i) {
			charCounter[str[i]]++;
		}
		buildHTree(str);
	}
	~CompressHuffTree() {
		delete[] charCounter;
		delHTree(root);
	}
	void write(std::ostream &out) const {
		Bitset<uchar> tree;
		std::string chars;
		chars.reserve(uniqueChars);
		buildBitsetFromTree(root, tree, chars);
		tree.write(out);
		int sz = chars.size();
		out.write(reinterpret_cast<const char *>(&sz), sizeof(sz));
		out.write(&chars[0], sz);
	}
	const int *getCharCountArr() const {
		return charCounter;
	}
private:
	int *charCounter;
	int uniqueChars;
	static const int alphabetSize = 256;
	struct Comp {
		bool operator()(const HuffNode *lhs, const HuffNode *rhs) const {
			return lhs->weight < rhs->weight;
		}
	};
	void buildHTree(const std::string &str) {
		Heap <HuffNode*, alphabetSize, Comp> maxPQ;
		for (int i = 0; i < alphabetSize; ++i) {
			if (charCounter[i] > 0) {
				maxPQ.push(new HuffNode(nullptr, nullptr, charCounter[i], static_cast<char>(i), true));
				uniqueChars++;
			}
		}
		uniqueChars = maxPQ.size();
		HuffNode *left, *right;
		while (maxPQ.size() > 1) {
			left = maxPQ.top();
			maxPQ.pop();
			right = maxPQ.top();
			maxPQ.pop();
			maxPQ.push(new HuffNode(left, right, left->weight + right->weight, '\0', false));
		}
		root = maxPQ.top();
	}

	void buildBitsetFromTree(const HuffNode *node, Bitset<uchar> &tree, std::string& chars) const {
		if (node->leaf) {
			chars += node->c;
			tree.push_back(1);
			return;
		}
		tree.push_back(0);
		buildBitsetFromTree(node->left, tree, chars);
		buildBitsetFromTree(node->right, tree, chars);
	}

	void delHTree(HuffNode *node) {
		if (!node->leaf) {
			delHTree(node->left);
			delHTree(node->right);
		}
		delete node;
	}
};

class DecompressHuffTree : public HuffTree {
	typedef unsigned char uchar;
public:
	DecompressHuffTree(const Bitset<uchar> &tree, const std::string &chars) {
		int idx = 0, charIdx = 0;
		root = buildTreeFromBitset(tree, chars, idx, charIdx);
	}
private:
	HuffNode* buildTreeFromBitset(const Bitset<uchar> &tree, const std::string &chars, int &idx, int &charIdx) {
		if (idx < tree.size()) {
			if (tree[idx++]) {
				return new HuffNode(nullptr, nullptr, 0, chars[charIdx++], true);
			}
			HuffNode *left = buildTreeFromBitset(tree, chars, idx, charIdx);
			HuffNode *right = buildTreeFromBitset(tree, chars, idx, charIdx);
			return new HuffNode(left, right, 0, '\0', false);
		}
		return nullptr;
	}
};
#endif