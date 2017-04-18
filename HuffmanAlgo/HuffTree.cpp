#include "HuffTree.h"


HuffTree::HuffTree() : root{} {}

HuffTree::~HuffTree() {
	delHTree(root);
}

void HuffTree::delHTree(HuffNode *node) {
	if (!node->leaf) {
		delHTree(node->left);
		delHTree(node->right);
	}
	delete node;
}


CompressTree::CompressTree(const std::string &str) {
  for (int i = 0; i < str.size(); ++i) {
    charCount[str[i]]++;
  }
  buildHTree(str);
}

void CompressTree::write(std::ostream &out) const {
  Bitset<uchar> tree;
  std::string chars;
  chars.reserve(uniqueChars);
  buildBitsetFromTree(root, tree, chars);
  tree.write(out);
  int sz = chars.size();
  out.write(reinterpret_cast<const char *>(&sz), sizeof(sz));
  out.write(&chars[0], sz);
}

std::vector<Bitset<ull> *> CompressTree::buildTable() const {
  std::vector<Bitset<ull> *> codeTable(alphabetSize);
  buildTable(root, Bitset<ull>(), codeTable);
  return codeTable;
}

Bitset<ull> CompressTree::compress(const std::string &str) const {
  std::vector<Bitset<ull> *> codeTable = buildTable();
  int codeBitSize = 0;
  for (int i = 0; i < alphabetSize; i++) {
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

void CompressTree::buildHTree(const std::string &str) {
  Heap<HuffNode *, alphabetSize, Comp> maxPQ;
  for (int i = 0; i < alphabetSize; ++i) {
    if (charCount[i] > 0) {
      maxPQ.push(new HuffNode(nullptr, nullptr, charCount[i],
                              static_cast<char>(i), true));
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
    maxPQ.push(
        new HuffNode(left, right, left->weight + right->weight, '\0', false));
  }
  root = maxPQ.top();
}

void CompressTree::buildBitsetFromTree(const HuffNode *node,
                                       Bitset<uchar> &tree,
                                       std::string &chars) const {
  if (node->leaf) {
    chars += node->c;
    tree.push_back(1);
    return;
  }
  tree.push_back(0);
  buildBitsetFromTree(node->left, tree, chars);
  buildBitsetFromTree(node->right, tree, chars);
}

void CompressTree::buildTable(const HuffTree::HuffNode *node, Bitset<ull> &curr,
                              std::vector<Bitset<ull> *> &codeTable) const {
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

DecompressTree::DecompressTree(const Bitset<uchar> &tree,
                               const std::string &chars) {
  int idx = 0, charIdx = 0;
  root = buildTreeFromBitset(tree, chars, idx, charIdx);
}

std::string DecompressTree::decompress(const Bitset<ull> &code) const {
  return decompress(root, code);
}

HuffTree::HuffNode *
DecompressTree::buildTreeFromBitset(const Bitset<uchar> &tree,
                                    const std::string &chars, int &idx,
                                    int &charIdx) {
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

std::string DecompressTree::decompress(HuffTree::HuffNode *root,
                                       const Bitset<ull> &code) const {
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
