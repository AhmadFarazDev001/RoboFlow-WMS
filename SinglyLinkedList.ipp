#pragma once
#include "SinglyLinkedList.h"
template <typename T>
SinglyLinkedList<T>::SinglyLinkedList()
{
    head = tail = nullptr;
    count = 0;
}

template <typename T>
SinglyLinkedList<T>::SinglyLinkedList(const SinglyLinkedList<T> &other) : head(nullptr), tail(nullptr), count(0)
{
    if (other.head == nullptr)
    {
        return;
    }

    ListNode<T> *p = other.head;
    ListNode<T> *t = new ListNode<T>(p->data);
    head = t;
    tail = head;
    p = p->next;
    while (p)
    {
        t = new ListNode<T>(p->data);
        tail->next = t;
        tail = t;
        p = p->next;
    }

    count = other.count;
}

template <typename T>
SinglyLinkedList<T> &SinglyLinkedList<T>::operator=(const SinglyLinkedList<T> &other)
{
    if (this != &other)
    {
        clear();
        if (other.head == nullptr)
            return *this;

        ListNode<T> *p = other.head;
        ListNode<T> *t = new ListNode<T>(p->data);
        head = t;
        tail = head;
        p = p->next;
        while (p)
        {
            t = new ListNode<T>(p->data);
            tail->next = t;
            tail = t;
            p = p->next;
        }

        count = other.count;
    }

    return *this;
}

template <typename T>
SinglyLinkedList<T>::~SinglyLinkedList()
{
    clear();
}

template <typename T>
void SinglyLinkedList<T>::clear()
{
    ListNode<T> *curr = head;
    ListNode<T> *next = nullptr;
    while (curr)
    {
        next = curr->next;
        delete curr;
        curr = next;
    }

    count = 0;
    head = tail = nullptr;
}

template <typename T>
void SinglyLinkedList<T>::insertFront(const T &value)
{
    ListNode<T> *t = new ListNode<T>(value);
    t->next = head;
    head = t;
    if (!tail)
        tail = head;
    count++;
}
template <typename T>
void SinglyLinkedList<T>::insertBack(const T &value)
{
    if (!head)
    {
        insertFront(value);
        return;
    }

    ListNode<T> *t = new ListNode<T>(value);
    tail->next = t;
    tail = t;
    count++;
}

template <typename T>
bool SinglyLinkedList<T>::removeFront()
{
    if (!head)
        return false;
    ListNode<T> *p = head;
    head = head->next;
    if (!head)
        tail = nullptr;
    count--;
    delete p;
    return true;
}

template <typename T>
bool SinglyLinkedList<T>::removeByValue(const T &value)
{
    if (!head)
        return false;

    if (head->data == value)
    {
        return removeFront();
    }

    ListNode<T> *p = head;
    ListNode<T> *q = nullptr;

    while (p && !(p->data == value))
    {
        q = p;
        p = p->next;
    }

    if (p)
    {
        q->next = p->next;
        if (p == tail)
            tail = q;

        count--;
        delete p;
        return true;
    }

    return false;
}
template <typename T>
template <typename Predicate>
bool SinglyLinkedList<T>::removeIf(Predicate pred)
{
    if (!head)
        return false;

    ListNode<T> *p = head;
    ListNode<T> *q = nullptr;

    while (p)
    {
        if (pred(p->data))
        {
            if (p == head)
            {
                return removeFront();
            }

            q->next = p->next;
            if (p == tail)
                tail = q;

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
T *SinglyLinkedList<T>::findByValue(const T &value)
{
    if (!head)
        return nullptr;

    ListNode<T> *p = head;
    while (p)
    {
        if (p->data == value)
        {
            return &(p->data);
        }
        p = p->next;
    }

    return nullptr;
}
template <typename T>
const T *SinglyLinkedList<T>::findByValue(const T &value) const
{
    if (!head)
        return nullptr;

    ListNode<T> *p = head;
    while (p)
    {
        if (p->data == value)
        {
            return &(p->data);
        }
        p = p->next;
    }

    return nullptr;
}

template <typename T>
template <typename Predicate>
T *SinglyLinkedList<T>::findIf(Predicate pred)
{
    if (!head)
        return nullptr;

    ListNode<T> *p = head;
    while (p)
    {
        if (pred(p->data))
        {
            return &(p->data);
        }
        p = p->next;
    }

    return nullptr;
}

template <typename T>
template <typename Predicate>
const T *SinglyLinkedList<T>::findIf(Predicate pred) const
{
    if (!head)
        return nullptr;

    ListNode<T> *p = head;
    while (p)
    {
        if (pred(p->data))
        {
            return &(p->data);
        }
        p = p->next;
    }

    return nullptr;
}

template <typename T>
ListNode<T> *SinglyLinkedList<T>::getHead() const
{
    return head;
}


template <typename T>
int SinglyLinkedList<T>::getSize() const
{
    return count;
}

template <typename T>
bool SinglyLinkedList<T>::isEmpty() const
{
    return head == nullptr;
}
