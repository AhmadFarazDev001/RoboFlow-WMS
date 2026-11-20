#pragma once

template <typename T>
struct StackNode {
    T data;
    StackNode<T>* next;

    StackNode(const T& value) : data(value), next(nullptr) {}
};

template <typename T>
class Stack {
private:
    StackNode<T>* topNode;
    int count;

public:
    Stack();
    Stack(const Stack<T>& other);
    Stack<T>& operator=(const Stack<T>& other);
    ~Stack();

    // Standard LIFO operations
    void push(const T& value);
    bool pop();
    T* top();
    const T* top() const;

    // State helpers
    int getSize() const;
    bool isEmpty() const;
    void clear();
};

#include "Stack.ipp"

