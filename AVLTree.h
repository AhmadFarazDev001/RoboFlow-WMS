#pragma once

template <typename K, typename V>
struct AVLNode {
    K key;
    V data;
    int height;
    AVLNode<K, V>* left;
    AVLNode<K, V>* right;

    AVLNode(const K& k, const V& val)
        : key(k), data(val), height(0), left(nullptr), right(nullptr) {}
    AVLNode(const K& k, const V& val, const int& h)
        : key(k), data(val), height(h), left(nullptr), right(nullptr) {}
};

template <typename K, typename V>
class AVLTree {
private:
    AVLNode<K, V>* root;
    int nodeCount;

    // Internal height and balance utilities
    int getHeight(AVLNode<K, V>* node) const;
    int getBalanceFactor(AVLNode<K, V>* node) const;
    int maxInt(int a, int b) const;

    // Rotations (LL, RR, LR, RL)
    AVLNode<K, V>* rotateRight(AVLNode<K, V>* x);
    AVLNode<K, V>* rotateLeft(AVLNode<K, V>* x);

    // Recursive core helpers
    AVLNode<K, V>* insertNode(AVLNode<K, V>* node, const K& key, const V& value, bool& inserted);
    AVLNode<K, V>* removeNode(AVLNode<K, V>* node, const K& key, bool& removed);
    AVLNode<K, V>* inSucc(AVLNode<K, V>* node) const;
    AVLNode<K, V>* searchNode(AVLNode<K, V>* node, const K& key) const;

    void clearTree(AVLNode<K, V>* node);
    AVLNode<K, V>* copyTree(const AVLNode<K, V>* node);

    template <typename Func>
    void inOrderHelper(AVLNode<K, V>* node, Func visit) const;

public:
    AVLTree();
    AVLTree(const AVLTree<K, V>& other);
    AVLTree<K, V>& operator=(const AVLTree<K, V>& other);
    ~AVLTree();

    // Core AVL operations
    bool insert(const K& key, const V& value); // Returns false if key already exists
    bool remove(const K& key);                 // Returns false if key not found
    V* search(const K& key);                   // Returns pointer to modify V in-place
    const V* search(const K& key) const;
    bool contains(const K& key) const;

    // Traversal & state helpers
    template <typename Func>
    void traverseInOrder(Func visit) const;

    int getSize() const;
    bool isEmpty() const;
    void clear();
};

#include "AVLTree.ipp"
