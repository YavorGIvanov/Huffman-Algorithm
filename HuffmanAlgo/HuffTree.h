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
	HuffTree() : root{} {}
    HuffTree(const HuffTree &other) = delete;
	HuffTree &operator=(const HuffTree &other) = delete;

protected:
	HuffNode *root;
};

class CompressHuffTree : public HuffTree {
	typedef unsigned char uchar;
	typedef unsigned long long ull;
public:
	CompressHuffTree(const std::string &str)  {
		charCount = new int[alphabetSize]();
		for (int i = 0; i < str.size(); ++i) {
			charCount[str[i]]++;
		}
		buildHTree(str);
	}
	~CompressHuffTree() {
		delete[] charCount;
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
	std::vector<Bitset<ull>*> buildTable() const {
		std::vector<Bitset<ull>*> codeTable(256);
		buildTable(root, Bitset<ull>(), codeTable);
		return codeTable;
	}

	Bitset<ull> compress(const std::string &str) const{
		std::vector<Bitset<ull>*> codeTable = buildTable();
		int codeBitSize = 0;
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
		for (int i = 0; i < codeTable.size(); i++) {
			delete codeTable[i];
		}
		return res;
	}
private:
	int *charCount;
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
			if (charCount[i] > 0) {
				maxPQ.push(new HuffNode(nullptr, nullptr, charCount[i], static_cast<char>(i), true));
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
	void buildTable(const HuffTree::HuffNode* node, Bitset<ull>& curr, std::vector<Bitset<ull>*>& codeTable) const{
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
	typedef unsigned long long ull;
public:
	DecompressHuffTree(const Bitset<uchar> &tree, const std::string &chars) {
		int idx = 0, charIdx = 0;
		root = buildTreeFromBitset(tree, chars, idx, charIdx);
	}
	std::string decompress(const Bitset<ull> &code) const {
		return decompress(root, code);
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
	std::string decompress(HuffTree::HuffNode *root, const Bitset<ull> &code) const{
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

};
#endif