#pragma once

template <typename T>
struct QueueNode {
    T data;
    QueueNode<T>* next;

    QueueNode(const T& value) : data(value), next(nullptr) {}
};

template <typename T>
class Queue {
private:
    QueueNode<T>* frontNode;
    QueueNode<T>* rearNode;
    int count;

public:
    Queue();
    Queue(const Queue<T>& other);
    Queue<T>& operator=(const Queue<T>& other);
    ~Queue();

    // Standard FIFO operations
    void enqueue(const T& value);
    bool dequeue();
    T* peekFront();
    const T* peekFront() const;

    // O(n) mid-queue removal for cancelled Pending orders
    bool removeByValue(const T& value);

    template <typename Predicate>
    bool removeIf(Predicate pred);

    // Non-destructive traversal & state helpers
    const QueueNode<T>* getFrontNode() const;
    int getSize() const;
    bool isEmpty() const;
    void clear();
};

#include "Queue.ipp"
