#pragma once

template <typename T>
struct ListNode {
    T data;
    ListNode<T>* next;

    ListNode(const T& value) : data(value), next(nullptr) {}
};

template <typename T>
class SinglyLinkedList {
private:
    ListNode<T>* head;
    ListNode<T>* tail;
    int count;

public:
    SinglyLinkedList();
    SinglyLinkedList(const SinglyLinkedList<T>& other);
    SinglyLinkedList<T>& operator=(const SinglyLinkedList<T>& other);
    ~SinglyLinkedList();

    // Insertion operations
    void insertFront(const T& value);
    void insertBack(const T& value);

    // Removal operations
    bool removeFront();
    bool removeByValue(const T& value);

    template <typename Predicate>
    bool removeIf(Predicate pred);

    // Search operations (returns pointer to allow in-place modification)
    T* findByValue(const T& value);
    const T* findByValue(const T& value) const;

    template <typename Predicate>
    T* findIf(Predicate pred);

    template <typename Predicate>
    const T* findIf(Predicate pred) const;

    // Iteration and state helpers
    ListNode<T>* getHead() const;
    int getSize() const;
    bool isEmpty() const;
    void clear();
};

#include "SinglyLinkedList.ipp"
