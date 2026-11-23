#pragma once
#include "library.h"

struct ListNode
{
    std::string data;
    ListNode *next;

    ListNode(const std::string &val) : data(val), next(nullptr) {}
};

// Simple singly linked list for location paths and iDs
class LinkedList
{
private:
    ListNode *head;
    ListNode *tail;
    int count;

public:
    LinkedList();
    LinkedList(const LinkedList &other);
    LinkedList &operator=(const LinkedList &other);
    ~LinkedList();

    void clear();
    void insertFront(const std::string &val);
    void insertBack(const std::string &val);
    bool removeByValue(const std::string &val);
    bool contains(const std::string &val) const;

    ListNode *getHead() const { return head; }
    int getSize() const { return count; }
    bool isEmpty() const { return head == nullptr; }
};

struct PairNode
{
    std::string key;
    std::string value;
    PairNode *next;

    PairNode(const std::string &k, const std::string &v) : key(k), value(v), next(nullptr) {}
};

// Linked list for keyvalue pairs {used for RackID -> Location id}
class PairList
{
private:
    PairNode *head;

public:
    PairList();
    ~PairList();

    PairList(const PairList &) = delete;
    PairList &operator=(const PairList &) = delete;

    void insertBack(const std::string &key, const std::string &value);
    bool exists(const std::string &key) const;
    std::string getValue(const std::string &key) const;
};

#include "LinkedList.ipp"