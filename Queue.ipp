#pragma once
#include "Queue.h"

template <typename T>
Queue<T>::Queue()
{
    frontNode = rearNode = nullptr;
    count = 0;
}

template <typename T>
Queue<T>::Queue(const Queue<T> &other):frontNode(nullptr),rearNode(nullptr),count(0)
{
    if (other.frontNode == nullptr)
        return;

    QueueNode<T> *p = other.frontNode;
    frontNode = new QueueNode<T>(p->data);
    rearNode = frontNode;
    p = p->next;
    while (p)
    {
        QueueNode<T> *t = new QueueNode<T>(p->data);
        rearNode->next = t;
        rearNode = t;
        p = p->next;
    }

    count = other.count;
}

template <typename T>
Queue<T> &Queue<T>::operator=(const Queue<T> &other)
{
    if (this != &other)
    {
        clear();
        if (other.frontNode == nullptr)
            return *this;

        QueueNode<T> *p = other.frontNode;
        frontNode = new QueueNode<T>(p->data);
        rearNode = frontNode;
        p = p->next;
        while (p)
        {
            QueueNode<T> *t = new QueueNode<T>(p->data);
            rearNode->next = t;
            rearNode = t;
            p = p->next;
        }

        count = other.count;
    }

    return *this;
}

template <typename T>
Queue<T>::~Queue()
{
    clear();
}

template <typename T>
void Queue<T>::clear()
{
    QueueNode<T> *curr = frontNode;
    QueueNode<T> *next = nullptr;
    while (curr)
    {
        next = curr->next;
        delete curr;
        curr = next;
    }

    frontNode = rearNode = nullptr;
    count = 0;
}

template <typename T>
void Queue<T>::enqueue(const T &value)
{
    QueueNode<T> *t = new QueueNode<T>(value);
    if (!rearNode)
    {
        frontNode = rearNode = t;
        count++;
        return;
    }

    rearNode->next = t;
    rearNode = t;
    count++;
}

template <typename T>
void Queue<T>::enqueueFront(const T &value)
{
    QueueNode<T> *t = new QueueNode<T>(value);
    t->next = frontNode;
    frontNode = t;
    if (!rearNode)
        rearNode = t;
    count++;
}

template <typename T>
bool Queue<T>::dequeue()
{
    if (isEmpty())
        return false;

    QueueNode<T> *p = frontNode;
    frontNode = frontNode->next;
    if (!frontNode)
        rearNode = nullptr;

    delete p;
    count--;
    return true;
}

template <typename T>
T *Queue<T>::peekFront()
{
    if (isEmpty())
        return nullptr;

    return &(frontNode->data);
}

template <typename T>
const T *Queue<T>::peekFront() const
{
    if (isEmpty())
        return nullptr;

    return &(frontNode->data);
}

template <typename T>
bool Queue<T>::removeByValue(const T &value)
{
    if (isEmpty())
        return false;

    if (frontNode->data == value)
    {
        return dequeue();
    }

    QueueNode<T> *p = frontNode;
    QueueNode<T> *q = nullptr;

    while (p && !(p->data == value))
    {
        q = p;
        p = p->next;
    }

    if (p)
    {
        q->next = p->next;
        if (p == rearNode)
            rearNode = q;

        count--;
        delete p;
        return true;
    }

    return false;
}

template <typename T>
template <typename Predicate>
bool Queue<T>::removeIf(Predicate pred)
{
    if (isEmpty())
        return false;

    QueueNode<T> *p = frontNode;
    QueueNode<T> *q = nullptr;

    while (p)
    {
        if (pred(p->data))
        {
            if (p == frontNode)
            {
                return dequeue();
            }

            q->next = p->next;
            if (p == rearNode)
                rearNode = q;

            count--;
            delete p;
            return true;
        }
        q = p;
        p = p->next;
    }

    return false;
}

template <typename T>
const QueueNode<T> *Queue<T>::getFrontNode() const
{
    return frontNode;
}

template <typename T>
int Queue<T>::getSize() const
{
    return count;
}

template <typename T>
bool Queue<T>::isEmpty() const
{
    return count == 0;
}
