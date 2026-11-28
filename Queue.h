// -- Written by Ahmad: Generic FIFO queue template class
#pragma once

template <typename T>
struct QueueNode {
    T data;
    QueueNode<T>* next;

    // -- Constructs a queue node with a value and null next pointer
    QueueNode(const T& value) : data(value), next(nullptr) {}
};

template <typename T>
class Queue {
private:
    QueueNode<T>* frontNode;
    QueueNode<T>* rearNode;
    int count;

public:
    // -- Initializes an empty queue
    Queue();
    // -- Deep copies all elements from another queue
    Queue(const Queue<T>& other);
    // -- Assigns a deep copy of another queue
    Queue<T>& operator=(const Queue<T>& other);
    // -- Frees all nodes allocated in the queue
    ~Queue();

    // -- Adds an item to the back of the queue
    void enqueue(const T& value);
    // -- Inserts an item at the front of the queue
    void enqueueFront(const T& value);
    // -- Removes the front item from the queue
    bool dequeue();
    // -- Returns a modifiable pointer to the front item
    T* peekFront();
    // -- Returns a read-only pointer to the front item
    const T* peekFront() const;

    // -- Removes the first node matching the given value
    bool removeByValue(const T& value);

    // -- Removes the first node matching the given predicate
    template <typename Predicate>
    bool removeIf(Predicate pred);

    // -- Returns the front node pointer for read-only traversal
    const QueueNode<T>* getFrontNode() const;
    // -- Returns the number of elements in the queue
    int getSize() const;
    // -- Checks whether the queue is empty
    bool isEmpty() const;
    // -- Deletes all nodes and resets the queue
    void clear();
};

#include "Queue.ipp"
