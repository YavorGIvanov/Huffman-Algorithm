#ifndef HUFFTREE_H
#define HUFFTREE_H
#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include "CharBuffer.h"
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

	HuffTree(const CharBuffer &str) : uniqueChars(), charCounter(), root(){
		charCounter = new int[alphabetSize]();
		for (int i = 0; i < str.size(); ++i) {
			charCounter[str[i]]++;
		}
		buildHTree(str);
	}
    HuffTree(const Bitset<uchar> &tree, const CharBuffer &chars)
            : uniqueChars(chars.size()), charCounter(), root() {
       int idx = 0, charIdx = 0;
       root = buildTreeFromBitset(tree, chars, idx, charIdx);
     }
    HuffTree(const HuffTree &other) = delete;
	HuffTree &operator=(const HuffTree &other) = delete;
	HuffNode *getRoot() const {
		return root;
	}
	const int *getCharCountArr() const {
		return charCounter;
	}
	void writeTreeToFile(FILE *out) const{
		Bitset<uchar> tree;
		CharBuffer chars(uniqueChars);
		buildBitsetFromTree(root, tree, chars);
		tree.writeToFile(out);
		chars.writeToFile(out);
	}
	~HuffTree() {
		delete[] charCounter;
		delHTree(root);
	}
private:
	HuffNode *root;
	int *charCounter;
	int uniqueChars;
	static const int alphabetSize = 256;

	struct Comp {
		bool operator()(const HuffNode *lhs, const HuffNode *rhs) const {
			return lhs->weight < rhs->weight;
		}
	};
	void buildHTree(const CharBuffer& str) {
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
	HuffNode* buildTreeFromBitset(const Bitset<uchar> &tree, const CharBuffer &chars, int &idx, int &charIdx) {
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
	void buildBitsetFromTree(const HuffNode *node, Bitset<uchar> &tree, CharBuffer& chars) const{
		if (node->leaf) {
			chars.push_back(node->c);
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
#endif