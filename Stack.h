// -- Written by Ahmad: Generic LIFO stack template class
#pragma once

template <typename T>
struct StackNode {
    T data;
    StackNode<T>* next;

    // -- Constructs a stack node with a value and null next pointer
    StackNode(const T& value) : data(value), next(nullptr) {}
};

template <typename T>
class Stack {
private:
    StackNode<T>* topNode;
    int count;

public:
    // -- Initializes an empty stack
    Stack();
    // -- Deep copies all elements from another stack
    Stack(const Stack<T>& other);
    // -- Assigns a deep copy of another stack
    Stack<T>& operator=(const Stack<T>& other);
    // -- Releases all nodes stored in the stack
    ~Stack();

    // -- Pushes a new item onto the top of the stack
    void push(const T& value);
    // -- Removes the top item from the stack
    bool pop();
    // -- Returns a modifiable pointer to the top element
    T* top();
    // -- Returns a read-only pointer to the top element
    const T* top() const;

    // -- Returns the current number of stacked items
    int getSize() const;
    // -- Checks whether the stack is empty
    bool isEmpty() const;
    // -- Removes all items from the stack
    void clear();
};

#include "Stack.ipp"

