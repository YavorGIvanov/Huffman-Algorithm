#ifndef BITSET_H
#define BITSET_H
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <type_traits>

template <typename blockType> class Bitset {
  static_assert(std::is_pod<blockType>::value,
                "The specified type is not POD type");
  static_assert(!std::is_signed<blockType>::value,
                "The specified type is not unsigned type");
  static_assert(CHAR_BIT == 8, "The byte size is not 8 bits");
  static_assert((sizeof(blockType) & (sizeof(blockType) - 1)) == 0,
                "The byte size of the specified type is not power of two");
  typedef int sizeType;

public:
  Bitset() : blockSize(0), bitSize(0), bitset(nullptr) {}

  explicit Bitset(sizeType mCap)
      : blockSize(ceil(mCap / bitsBTf())), bitSize(0) {
    bitset = new blockType[blockSize];
    memset(bitset, 0, blockSize * sizeof(blockType));
  }

  explicit Bitset(FILE *in) {
    if (!in) {
      return;
    }
    fread(&bitSize, sizeof(bitSize), 1, in);
    blockSize = ceil(bitSize / bitsBTf());
    bitset = new blockType[blockSize];
    fread(bitset, sizeof(blockType), blockSize, in);
  }

  Bitset(const Bitset<blockType> &other)
      : blockSize(other.blockSize), bitSize(other.bitSize) {
    bitset = new blockType[other.blockSize];
    memcpy(bitset, other.bitset, blockSize * sizeof(blockType));
  }
  Bitset &operator=(Bitset<blockType> other) {
    blockType *temp = other.bitset;
    other.bitset = bitset;
    bitset = temp;
    blockSize = other.blockSize;
    bitSize = other.bitSize;
    return *this;
  }
  ~Bitset() { 
	  delete[] bitset; 
  }

  void reserve(sizeType newBitCap) {
    sizeType newblockSize = ceil(newBitCap / bitsBTf());
    if (newblockSize > blockSize) {
      blockType *newBitset = new blockType[newblockSize];
      memset(newBitset, 0, newblockSize * sizeof(blockType));
      memcpy(newBitset, bitset, blockSize * sizeof(blockType));
      blockSize = newblockSize;
      delete[] bitset;
      bitset = newBitset;
    }
  }

  void resize(sizeType newBitSize) {
    sizeType newblockSize = ceil(newBitSize / bitsBTf());
    if (newblockSize > blockSize) {
      reserve(newBitSize);
    } 
	else if (newblockSize < blockSize) {
      for (int i = newblockSize; i < blockSize; ++i) {
        bitset[i] = 0;
      }
      for (int i = newBitSize; i < bitsBT(); ++i) {
        reset(i);
      }
    }
    bitSize = newBitSize;
  }
  Bitset &set(sizeType idx, bool val = true) {
    assert(idx < bitSize);
    bitset[blockIndex(idx)] =
        val ? (bitset[blockIndex(idx)] | (1ULL << bitIndex(idx)))
            : (bitset[blockIndex(idx)] & ~(1ULL << bitIndex(idx)));
    return *this;
  }

  Bitset &set() {
    memset(bitset, -1, blockIndex(bitSize) * sizeof(blockType));
    const sizeType bitsLeftToSet = bitSize & (bitsBT() - 1);
    bitset[blockIndex(bitSize)] |=
        ((1ULL << bitsLeftToSet) - 1 << (bitsBT() - bitsLeftToSet));
    return *this;
  }

  Bitset &reset(sizeType idx) {
    assert(idx < bitSize);
    bitset[blockIndex(idx)] &= ~(1ULL << bitIndex(idx));
    return *this;
  }

  Bitset &reset() {
    memset(bitset, 0, blockSize * sizeof(blockType));
    return *this;
  }

  Bitset &flip(sizeType idx) {
    assert(idx < bitSize);
    bitset[blockIndex(idx)] ^= (1ULL << bitIndex(idx));
    return *this;
  }

  Bitset &flip() {
    for (int i = 0; i < blockIndex(bitSize); ++i) {
      bitset[i] = ~bitset[i];
    }
    const sizeType bitsLeftToFlip = bitSize & (bitsBT() - 1);
    for (int i = 0; i < bitsLeftToFlip; ++i) {
      flip(bitSize - i - 1);
    }
    return *this;
  }

  void push_back(bool val) {
    if (blockIndex(bitSize) == blockSize) {
      reserve((blockSize + 1) * bitsBT());
    }
    if (val) {
      bitset[blockIndex(bitSize)] |= (1ULL << bitIndex(bitSize));
    }
    ++bitSize;
  }

  void pop_back() {
    assert(bitSize > 0);
    reset(bitSize - 1);
    --bitSize;
  }

  void writeToFile(FILE *out) const {
    if (!out) {
      return;
    }
    fwrite(&bitSize, sizeof(bitSize), 1, out);
    fwrite(bitset, (ceil(bitSize / bitsBTf()) * sizeof(blockType)), 1, out);
  }

  void writeToFileNoSz(FILE *out) const {
    if (!out) {
      return;
    }
    fwrite(bitset, sizeof(blockType), blockSize, out);
  }

  bool empty() const { 
	  return bitSize == 0; 
  }

  sizeType Bitset::size() const { 
	  return bitSize; 
  }

  blockType getBlock(sizeType idx) const { 
	  return bitset[idx]; 
  }

  void push_back_block(blockType newBlock) {
    reserve(bitSize + 2 * bitsBT());
    int freeBits = bitsBT() - (bitSize & (bitsBT() - 1));
    bitset[blockSize - 2] |= (newBlock >> (bitsBT() - freeBits));
    bitset[blockSize - 1] |= (newBlock << (freeBits));
    bitSize += bitsBT();
  }

  bool operator[](sizeType idx) const {
    return bitset[blockIndex(idx)] & (1ULL << bitIndex(idx));
  }

  Bitset &operator+=(const Bitset &other) {
    if ((bitSize & (bitsBT() - 1)) == 0) {
	  int actualBlockSize = floor(bitSize / bitsBTf());
      memcpy(bitset + actualBlockSize, other.bitset,
             other.blockSize * sizeof(blockType));
    } 
	else {
      int blockIdx = blockIndex(bitSize);
      int freeBits = bitsBT() - (bitSize & (bitsBT() - 1));
      int blkSz = other.blockSize - 1;
      for (int i = 0; i < blkSz; ++i) {
        blockType currBlock = other.bitset[i];
        bitset[blockIdx + i] |= (currBlock >> (bitsBT() - freeBits));
        bitset[blockIdx + i + 1] |= (currBlock << (freeBits));
      }
      bitset[blockIdx + blkSz] |=
          (other.bitset[blkSz] >> (bitsBT() - freeBits));
      if (freeBits < (other.bitSize & (bitsBT() - 1))) {
        bitset[blockIdx + blkSz + 1] |= (other.bitset[blkSz] << (freeBits));
      }
    }
    bitSize += other.size();
    return *this;
  }

  friend Bitset operator+(Bitset lhs, const Bitset &rhs) {
    lhs += rhs;
    return lhs;
  }

private:
  blockType *bitset;
  sizeType bitSize, blockSize;
  static constexpr sizeType bitsBT() { return sizeof(blockType) * 8; }
  static constexpr float bitsBTf() { return sizeof(blockType) * 8.0f; }
  static constexpr sizeType blockIndex(sizeType pos) { return pos / bitsBT(); }
  static constexpr sizeType bitIndex(sizeType pos) {
    return (bitsBT() - 1) - (pos & (bitsBT() - 1));
  }
};

#endif
