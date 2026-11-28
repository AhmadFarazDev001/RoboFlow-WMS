// -- Written by Ahmad: Generic singly linked list template class
#pragma once

template <typename T>
struct ListNode
{
    T data;
    ListNode<T> *next;

    // -- Initializes a list node with a value and null next pointer
    ListNode(const T &value) : data(value), next(nullptr) {}
};

template <typename T>
class SinglyLinkedList
{
private:
    ListNode<T> *head;
    ListNode<T> *tail;
    int count;

public:
    // -- Initializes an empty singly linked list
    SinglyLinkedList();
    // -- Deep copies all nodes from another singly linked list
    SinglyLinkedList(const SinglyLinkedList<T> &other);
    // -- Replaces list contents with a deep copy of another list
    SinglyLinkedList<T> &operator=(const SinglyLinkedList<T> &other);
    // -- Releases all nodes allocated in the list
    ~SinglyLinkedList();

    // -- Inserts a new element at the front of the list
    void insertFront(const T &value);
    // -- Appends a new element at the back of the list
    void insertBack(const T &value);

    // -- Removes the first node from the list
    bool removeFront();
    // -- Removes the first node matching the given value
    bool removeByValue(const T &value);

    // -- Removes the first node satisfying the given predicate
    template <typename Predicate>
    bool removeIf(Predicate pred);

    // -- Searches for a value and returns a modifiable pointer
    T *findByValue(const T &value);
    // -- Searches for a value and returns a read-only pointer
    const T *findByValue(const T &value) const;

    // -- Finds the first matching node by predicate and returns a modifiable pointer
    template <typename Predicate>
    T *findIf(Predicate pred);

    // -- Finds the first matching node by predicate and returns a read-only pointer
    template <typename Predicate>
    const T *findIf(Predicate pred) const;

    // -- Returns the head node pointer for traversal
    ListNode<T> *getHead() const;
    // -- Returns the number of elements in the list
    int getSize() const;
    // -- Checks whether the list has no elements
    bool isEmpty() const;
    // -- Deletes all nodes and resets the list to empty
    void clear();
};

#include "SinglyLinkedList.ipp"
