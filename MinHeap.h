#pragma once
#include "library.h"

struct HeapNode
{
    int weight;
    int hops;
    int index;
};

class MinHeap
{
private:
    HeapNode *arr;
    int cap;
    int count;

    bool isBetter(const HeapNode &a, const HeapNode &b) const;
    void resize();
    void heapifyUp(int i);
    void heapifyDown(int i);

public:
    MinHeap(int initialCap = 16);
    ~MinHeap();

    bool isEmpty() const { return count == 0; }
    void push(int weight, int hops, int index);
    HeapNode pop();
};

#include "MinHeap.ipp"