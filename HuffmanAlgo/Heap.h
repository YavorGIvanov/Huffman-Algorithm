#ifndef HEAP_H
#define HEAP_H
#include <cassert>

template <typename T, size_t capacity, class Compare = std::less<T>>
class Heap {
public:
	Heap(): sz(1) {}
	void push(const T &data) {
		assert(sz < capacity+1);
		heap[sz++] = data;
		int index = sz-1, parent = index / 2;
		while (parent > 0 && Compare()(data, heap[parent])) {
			heap[index] = heap[parent];
			heap[parent] = data;
			index = parent;
			parent /= 2;
		}
	}
	void pop() {
		assert(sz > 1);
		--sz;
		heap[1] = heap[sz];
		int current = 1, child = 2;
		T temp;
		while (child < sz) {
			child = (child+1 < sz && Compare()(heap[child+1], heap[child])) ? child + 1 : child;
			if (Compare()(heap[current], heap[child])){
				break;
			}
			temp = heap[current];
			heap[current] = heap[child];
			heap[child] = temp;
			current = child;
			child = 2 * current;
		}
	}
	T const top() const {
		assert(sz != 1);
		return heap[1];
	}
	int size() const {
		return sz-1;
	}
	bool empty() const {
		return sz == 1;
	}
private:
	T heap[capacity+1];
	int sz;
};
#endif