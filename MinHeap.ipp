#pragma once
#include "MinHeap.h"

bool MinHeap::isBetter(const HeapNode &a, const HeapNode &b) const
{
    if (a.weight != b.weight)
        return a.weight < b.weight;
    return a.hops < b.hops;
}

void MinHeap::resize()
{
    cap *= 2;
    HeapNode *newArr = new HeapNode[cap];
    for (int i = 0; i < count; i++)
        newArr[i] = arr[i];

    delete[] arr;
    arr = newArr;
}

void MinHeap::heapifyUp(int i)
{
    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (isBetter(arr[i], arr[parent]))
        {
            HeapNode temp = arr[i];
            arr[i] = arr[parent];
            arr[parent] = temp;
            i = parent;
        }
        else
        {
            break;
        }
    }
}

void MinHeap::heapifyDown(int i)
{
    while (true)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < count && isBetter(arr[left], arr[smallest]))
            smallest = left;
        if (right < count && isBetter(arr[right], arr[smallest]))
            smallest = right;

        if (smallest != i)
        {
            HeapNode temp = arr[i];
            arr[i] = arr[smallest];
            arr[smallest] = temp;
            i = smallest;
        }
        else
        {
            break;
        }
    }
}

MinHeap::MinHeap(int initialCap)
{
    cap = (initialCap > 0) ? initialCap : 16;
    count = 0;
    arr = new HeapNode[cap];
}

MinHeap::~MinHeap()
{
    delete[] arr;
}

void MinHeap::push(int weight, int hops, int index)
{
    if (count == cap)
        resize();

    arr[count].weight = weight;
    arr[count].hops = hops;
    arr[count].index = index;
    heapifyUp(count);
    count++;
}

HeapNode MinHeap::pop()
{
    HeapNode top = arr[0];
    arr[0] = arr[count - 1];
    count--;
    if (count > 0)
        heapifyDown(0);
    return top;
}
