#pragma once
#include "Stack.h"

template <typename T>
Stack<T>::Stack()
{
    topNode = nullptr;
    count = 0;
}

template <typename T>
Stack<T>::Stack(const Stack<T> &other) : topNode(nullptr), count(0)
{
    if (other.topNode == nullptr)
        return;

    StackNode<T> *p = other.topNode;
    StackNode<T> *t = new StackNode<T>(p->data);
    topNode = t;
    StackNode<T> *tail = topNode;
    p = p->next;
    while (p)
    {
        t = new StackNode<T>(p->data);
        tail->next = t;
        tail = t;
        p = p->next;
    }

    count = other.count;
}

template <typename T>
Stack<T> &Stack<T>::operator=(const Stack<T> &other)
{
    if (this != &other)
    {
        clear();
        if (!other.topNode)
            return *this;

        StackNode<T> *p = other.topNode;
        StackNode<T> *t = new StackNode<T>(p->data);
        topNode = t;
        StackNode<T> *tail = topNode;
        p = p->next;
        while (p)
        {
            t = new StackNode<T>(p->data);
            tail->next = t;
            tail = t;
            p = p->next;
        }

        count = other.count;
    }

    return *this;
}

template <typename T>
Stack<T>::~Stack()
{
    clear();
}

template <typename T>
void Stack<T>::clear()
{
    StackNode<T> *curr = topNode;
    StackNode<T> *next = nullptr;
    while (curr)
    {
        next = curr->next;
        delete curr;
        curr = next;
    }

    count = 0;
    topNode = nullptr;
}
template <typename T>
void Stack<T>::push(const T &value)
{
    StackNode<T> *t = new StackNode<T>(value);
    t->next = topNode;
    topNode = t;
    count++;
}

template <typename T>
bool Stack<T>::pop()
{
    if (isEmpty())
        return false;

    StackNode<T> *p = topNode;
    topNode = topNode->next;
    count--;
    delete p;
    return true;
}

template <typename T>
T *Stack<T>::top()
{
    if (isEmpty())
        return nullptr;
    return &(topNode->data);
}

template <typename T>
const T *Stack<T>::top() const
{
    if (isEmpty())
        return nullptr;
    return &(topNode->data);
}

template <typename T>
int Stack<T>::getSize() const
{
    return count;
}

template <typename T>
bool Stack<T>::isEmpty() const
{
    return count == 0;
}