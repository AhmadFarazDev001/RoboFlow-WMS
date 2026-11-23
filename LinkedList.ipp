#pragma once
#include "LinkedList.h"

// ~~~~ linkedList ~~~~

inline LinkedList::LinkedList() : head(nullptr), tail(nullptr), count(0) {}

inline LinkedList::LinkedList(const LinkedList &other) : head(nullptr), tail(nullptr), count(0)
{
    ListNode *curr = other.head;
    while (curr != nullptr)
    {
        insertBack(curr->data);
        curr = curr->next;
    }
}

inline LinkedList &LinkedList::operator=(const LinkedList &other)
{
    if (this != &other)
    {
        clear();
        ListNode *curr = other.head;
        while (curr != nullptr)
        {
            insertBack(curr->data);
            curr = curr->next;
        }
    }
    return *this;
}

inline LinkedList::~LinkedList()
{
    clear();
}

inline void LinkedList::clear()
{
    ListNode *curr = head;
    while (curr != nullptr)
    {
        ListNode *nextNode = curr->next;
        delete curr;
        curr = nextNode;
    }
    head = tail = nullptr;
    count = 0;
}

inline void LinkedList::insertFront(const std::string &val)
{
    ListNode *node = new ListNode(val);
    node->next = head;
    head = node;
    if (!tail)
        tail = head;
    count++;
}

inline void LinkedList::insertBack(const std::string &val)
{
    ListNode *node = new ListNode(val);
    if (!head)
    {
        head = tail = node;
    }
    else
    {
        tail->next = node;
        tail = node;
    }
    count++;
}

inline bool LinkedList::removeByValue(const std::string &val)
{
    if (!head)
        return false;

    ListNode *curr = head;
    ListNode *prev = nullptr;
    while (curr != nullptr)
    {
        if (curr->data == val)
        {
            if (prev == nullptr)
                head = curr->next;
            else
                prev->next = curr->next;

            if (curr == tail)
                tail = prev;

            delete curr;
            count--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

inline bool LinkedList::contains(const std::string &val) const
{
    ListNode *curr = head;
    while (curr != nullptr)
    {
        if (curr->data == val)
            return true;
        curr = curr->next;
    }
    return false;
}

// ~~~ PairList ~~~

inline PairList::PairList() : head(nullptr) {}

inline PairList::~PairList()
{
    PairNode *curr = head;
    while (curr != nullptr)
    {
        PairNode *nextNode = curr->next;
        delete curr;
        curr = nextNode;
    }
}

inline void PairList::insertBack(const std::string &key, const std::string &value)
{
    PairNode *node = new PairNode(key, value);
    if (!head)
    {
        head = node;
        return;
    }
    PairNode *curr = head;
    while (curr->next != nullptr)
        curr = curr->next;
    curr->next = node;
}

inline bool PairList::exists(const std::string &key) const
{
    PairNode *curr = head;
    while (curr != nullptr)
    {
        if (curr->key == key)
            return true;
        curr = curr->next;
    }
    return false;
}

inline std::string PairList::getValue(const std::string &key) const
{
    PairNode *curr = head;
    while (curr != nullptr)
    {
        if (curr->key == key)
            return curr->value;
        curr = curr->next;
    }
    return "";
}