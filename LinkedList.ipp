#pragma once
#include "LinkedList.h"

LinkedList::LinkedList() : head(nullptr), tail(nullptr), count(0) {}

LinkedList::LinkedList(const LinkedList &other) : head(nullptr), tail(nullptr), count(0)
{
    ListNode *curr = other.head;
    while (curr != nullptr)
    {
        insertBack(curr->data);
        curr = curr->next;
    }
}

LinkedList &LinkedList::operator=(const LinkedList &other)
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

LinkedList::~LinkedList()
{
    clear();
}

void LinkedList::clear()
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

void LinkedList::insertFront(const std::string &val)
{
    ListNode *node = new ListNode(val);
    node->next = head;
    head = node;
    if (tail == nullptr)
        tail = head;
    count++;
}

void LinkedList::insertBack(const std::string &val)
{
    ListNode *node = new ListNode(val);
    if (head == nullptr)
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

bool LinkedList::removeByValue(const std::string &val)
{
    if (head == nullptr)
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

bool LinkedList::contains(const std::string &val) const
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

// PairList functions

PairList::PairList() : head(nullptr) {}

PairList::~PairList()
{
    PairNode *curr = head;
    while (curr != nullptr)
    {
        PairNode *nextNode = curr->next;
        delete curr;
        curr = nextNode;
    }
}

void PairList::insertBack(const std::string &key, const std::string &value)
{
    PairNode *node = new PairNode(key, value);
    if (head == nullptr)
    {
        head = node;
        return;
    }
    PairNode *curr = head;
    while (curr->next != nullptr)
        curr = curr->next;
    curr->next = node;
}

bool PairList::exists(const std::string &key) const
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

std::string PairList::getValue(const std::string &key) const
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
