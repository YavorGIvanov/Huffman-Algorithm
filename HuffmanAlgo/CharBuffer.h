#ifndef CHARBUFFER_H
#define CHARBUFFER_H
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <cassert>

class CharBuffer {
	typedef unsigned char uchar;
public:
	explicit CharBuffer(int mCapacity) : capacity(mCapacity+1), sz(0) {
		buffer = new uchar[capacity];
	}
	CharBuffer(const uchar *str, int sz): sz(sz) {
		capacity = sz + 1;
		buffer = new uchar[capacity];
		memcpy(buffer, str, capacity);
	}
	explicit CharBuffer(FILE *in, int sz): sz(sz), capacity(sz+1) {
		buffer = new uchar[capacity];
		fread(buffer, 1, sz, in);
		buffer[sz] = '\0';
	}
	CharBuffer(const CharBuffer &other) : capacity(other.capacity), sz(other.sz) {
		buffer = new uchar[capacity];
		memcpy(buffer, other.buffer, sz+1);
	}
	CharBuffer &operator=(CharBuffer other) {
		sz = other.sz;
		capacity = other.capacity;
		uchar *temp = buffer;
		buffer = other.buffer;
		other.buffer = temp;
		return *this;
	}
	~CharBuffer() {
		delete[] buffer;
	}
	void writeToFile(FILE *out) const{
		fwrite(&sz, sizeof(sz), 1, out);
		fwrite(buffer, 1, sz, out);
	}
	void writeToFileNoSz(FILE *out) const {
		fwrite(buffer, 1, sz, out);
	}
	int size() const {
		return sz;
	}
	const uchar *getBuffer() const {
		return buffer;
	}
	const uchar &operator[](int idx) const {
		assert(idx < sz);
		return buffer[idx];
	}
	uchar &operator[](int idx) {
		assert(idx < sz);
		return buffer[idx];
	}
	void push_back(char newChar) {
		if (sz == capacity-1) {
			capacity *= 2;
			uchar *temp = new uchar[capacity];
			memcpy(temp, buffer, sz + 1);
			delete[] buffer;
			buffer = temp;
		}
		buffer[sz++] = newChar;
		buffer[sz] = '\0';
	}
private:
	uchar *buffer;
	int sz, capacity;
};
#endif