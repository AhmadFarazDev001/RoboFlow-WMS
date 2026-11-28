// -- Written by Ahmad: Generic self-balancing AVL tree template class
#pragma once

template <typename K, typename V>
struct AVLNode {
    K key;
    V data;
    int height;
    AVLNode<K, V>* left;
    AVLNode<K, V>* right;

    // -- Constructs an AVL node with default height zero
    AVLNode(const K& k, const V& val)
        : key(k), data(val), height(0), left(nullptr), right(nullptr) {}
    // -- Constructs an AVL node with an explicit initial height
    AVLNode(const K& k, const V& val, const int& h)
        : key(k), data(val), height(h), left(nullptr), right(nullptr) {}
};

template <typename K, typename V>
class AVLTree {
private:
    AVLNode<K, V>* root;
    int nodeCount;

    // -- Returns the height of the given node or -1 if null
    int getHeight(AVLNode<K, V>* node) const;
    // -- Computes the height difference between left and right subtrees
    int getBalanceFactor(AVLNode<K, V>* node) const;
    // -- Returns the larger of two integers
    int maxInt(int a, int b) const;

    // -- Performs a right rotation to rebalance the subtree
    AVLNode<K, V>* rotateRight(AVLNode<K, V>* x);
    // -- Performs a left rotation to rebalance the subtree
    AVLNode<K, V>* rotateLeft(AVLNode<K, V>* x);

    // -- Recursively inserts a key-value pair and rebalances the tree
    AVLNode<K, V>* insertNode(AVLNode<K, V>* node, const K& key, const V& value, bool& inserted);
    // -- Recursively removes a node by key and rebalances the tree
    AVLNode<K, V>* removeNode(AVLNode<K, V>* node, const K& key, bool& removed);
    // -- Finds the inorder successor node in a subtree
    AVLNode<K, V>* inSucc(AVLNode<K, V>* node) const;
    // -- Recursively locates a node matching the given key
    AVLNode<K, V>* searchNode(AVLNode<K, V>* node, const K& key) const;

    // -- Recursively deallocates all nodes in a subtree
    void clearTree(AVLNode<K, V>* node);
    // -- Recursively creates a deep copy of a subtree
    AVLNode<K, V>* copyTree(const AVLNode<K, V>* node);

    // -- Visits subtree nodes in sorted key order
    template <typename Func>
    void inOrderHelper(AVLNode<K, V>* node, Func visit) const;

public:
    // -- Initializes an empty AVL tree
    AVLTree();
    // -- Deep copies another AVL tree
    AVLTree(const AVLTree<K, V>& other);
    // -- Assigns a deep copy of another AVL tree
    AVLTree<K, V>& operator=(const AVLTree<K, V>& other);
    // -- Frees all nodes in the AVL tree
    ~AVLTree();

    // -- Inserts a new key-value pair if the key does not exist
    bool insert(const K& key, const V& value);
    // -- Removes the entry matching the specified key
    bool remove(const K& key);
    // -- Searches for a key and returns a modifiable value pointer
    V* search(const K& key);
    // -- Searches for a key and returns a read-only value pointer
    const V* search(const K& key) const;
    // -- Checks whether the specified key exists in the tree
    bool contains(const K& key) const;

    // -- Applies a visitor callback to all entries in key order
    template <typename Func>
    void traverseInOrder(Func visit) const;

    // -- Returns the total number of stored entries
    int getSize() const;
    // -- Checks whether the tree contains no entries
    bool isEmpty() const;
    // -- Removes all entries from the tree
    void clear();
};

#include "AVLTree.ipp"
