#pragma once

#include "AVLTree.h"

template <typename K, typename V>
AVLTree<K, V>::AVLTree()
{
    root = nullptr;
    nodeCount = 0;
}

template <typename K, typename V>
AVLTree<K, V>::AVLTree(const AVLTree<K, V> &other) : root(nullptr), nodeCount(0)
{
    if (!other.root)
        return;
    root = copyTree(other.root);
    nodeCount = other.nodeCount;
}

template <typename K, typename V>
AVLTree<K, V> &AVLTree<K, V>::operator=(const AVLTree<K, V> &other)
{
    if (this != &other)
    {
        clear();
        if (!other.root)
            return *this;

        root = copyTree(other.root);
        nodeCount = other.nodeCount;
    }

    return *this;
}

template <typename K, typename V>
AVLTree<K, V>::~AVLTree()
{
    clearTree(root);
    root = nullptr;
    nodeCount = 0;
}

template <typename K, typename V>
void AVLTree<K, V>::clearTree(AVLNode<K, V> *node)
{
    if (node == nullptr)
        return;

    clearTree(node->left);
    clearTree(node->right);
    delete node;
}
template <typename K, typename V>
AVLNode<K, V> *AVLTree<K, V>::copyTree(const AVLNode<K, V> *node)
{
    if (node == nullptr)
        return nullptr;

    AVLNode<K, V> *p = new AVLNode<K, V>(node->key, node->data, node->height);
    p->left = copyTree(node->left);
    p->right = copyTree(node->right);
    return p;
}

template <typename K, typename V>
int AVLTree<K, V>::getHeight(AVLNode<K, V> *node) const
{
    if (node == nullptr)
        return -1;

    return node->height;
}

template <typename K, typename V>
int AVLTree<K, V>::getBalanceFactor(AVLNode<K, V> *node) const
{
    if (!node)
        return 0;
    return (getHeight(node->left) - getHeight(node->right));
}

template <typename K, typename V>
int AVLTree<K, V>::maxInt(int a, int b) const
{
    return (a > b) ? a : b;
}

template <typename K, typename V>
AVLNode<K, V> *AVLTree<K, V>::rotateRight(AVLNode<K, V> *x)
{
    AVLNode<K, V> *xl = x->left;
    AVLNode<K, V> *xlr = xl->right;

    x->left = xlr;
    xl->right = x;

    x->height = maxInt(getHeight(x->left), getHeight(x->right)) + 1;
    xl->height = maxInt(getHeight(xl->left), getHeight(xl->right)) + 1;

    return xl;
}

template <typename K, typename V>
AVLNode<K, V> *AVLTree<K, V>::rotateLeft(AVLNode<K, V> *x)
{
    AVLNode<K, V> *xr = x->right;
    AVLNode<K, V> *xrl = xr->left;

    x->right = xrl;
    xr->left = x;

    x->height = maxInt(getHeight(x->left), getHeight(x->right)) + 1;
    xr->height = maxInt(getHeight(xr->left), getHeight(xr->right)) + 1;

    return xr;
}

template <typename K, typename V>
AVLNode<K, V> *AVLTree<K, V>::insertNode(AVLNode<K, V> *node, const K &key, const V &value, bool &inserted)
{
    if (node == nullptr)
    {
        AVLNode<K, V> *t = new AVLNode<K, V>(key, value);
        inserted = true;
        nodeCount++;
        return t;
    }

    if (key < node->key)
    {
        node->left = insertNode(node->left, key, value, inserted);
    }
    else if (key > node->key)
    {
        node->right = insertNode(node->right, key, value, inserted);
    }
    else
    {
        inserted = false;
        return node;
    }

    node->height = maxInt(getHeight(node->left), getHeight(node->right)) + 1;

    int bf = getBalanceFactor(node);

    // -- Left heavy
    if (bf > 1)
    {
        if (getBalanceFactor(node->left) >= 0)
        {
            return rotateRight(node);
        }
        else
        {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
    }
    else if (bf < -1) // -- Right heavy
    {
        if (getBalanceFactor(node->right) <= 0)
        {
            return rotateLeft(node);
        }
        else
        {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
    }

    return node;
}

template <typename K, typename V>
AVLNode<K, V> *AVLTree<K, V>::removeNode(AVLNode<K, V> *node, const K &key, bool &removed)
{
    if (node == nullptr)
    {
        removed = false;
        return nullptr;
    }

    if (key < node->key)
    {
        node->left = removeNode(node->left, key, removed);
    }
    else if (key > node->key)
    {
        node->right = removeNode(node->right, key, removed);
    }
    else
    {
        if (!node->left)
        {
            AVLNode<K, V> *t = node->right;
            delete node;
            node = t;
            nodeCount--;
        }
        else if (!node->right)
        {
            AVLNode<K, V> *t = node->left;
            delete node;
            node = t;
            nodeCount--;
        }
        else
        {
            AVLNode<K, V> *q = inSucc(node->right);
            node->key = q->key;
            node->data = q->data;
            node->right = removeNode(node->right, q->key, removed);
        }
        removed = true;
    }

    if (node)
    {

        node->height = maxInt(getHeight(node->left), getHeight(node->right)) + 1;

        int bf = getBalanceFactor(node);

        // -- Left heavy
        if (bf > 1)
        {
            if (getBalanceFactor(node->left) >= 0)
            {
                return rotateRight(node);
            }
            else
            {
                node->left = rotateLeft(node->left);
                return rotateRight(node);
            }
        }
        else if (bf < -1) // -- Right heavy
        {
            if (getBalanceFactor(node->right) <= 0)
            {
                return rotateLeft(node);
            }
            else
            {
                node->right = rotateRight(node->right);
                return rotateLeft(node);
            }
        }
    }

    return node;
}

template <typename K, typename V>
AVLNode<K, V> *AVLTree<K, V>::inSucc(AVLNode<K, V> *node) const
{
    while (node && node->left)
    {
        node = node->left;
    }

    return node;
}

template <typename K, typename V>
AVLNode<K, V> *AVLTree<K, V>::searchNode(AVLNode<K, V> *node, const K &key) const
{
    if (node == nullptr)
        return nullptr;

    if (key < node->key)
    {
        return searchNode(node->left, key);
    }
    else if (key > node->key)
    {
        return searchNode(node->right, key);
    }
    else
    {
        return node;
    }
}

template <typename K, typename V>
bool AVLTree<K, V>::insert(const K &key, const V &value)
{
    bool inserted = false;
    root = insertNode(root, key, value, inserted);
    return inserted;
}

template <typename K, typename V>
bool AVLTree<K, V>::remove(const K &key)
{
    bool removed = false;
    root = removeNode(root, key, removed);
    return removed;
}

template <typename K, typename V>
V *AVLTree<K, V>::search(const K &key)
{
    AVLNode<K, V> *p = searchNode(root, key);
    return p ? &(p->data) : nullptr;
}

template <typename K, typename V>
const V *AVLTree<K, V>::search(const K &key) const
{
    AVLNode<K, V> *p = searchNode(root, key);
    return p ? &(p->data) : nullptr;
}

template <typename K, typename V>
bool AVLTree<K, V>::contains(const K &key) const
{
    AVLNode<K, V> *p = searchNode(root, key);
    if (p)
        return true;
    else
        return false;
}

template <typename K, typename V>
int AVLTree<K, V>::getSize() const
{
    return nodeCount;
}

template <typename K, typename V>
bool AVLTree<K, V>::isEmpty() const
{
    return nodeCount == 0;
}

template <typename K, typename V>
void AVLTree<K, V>::clear()
{
    clearTree(root);
    root = nullptr;
    nodeCount = 0;
}

template <typename K, typename V>
template <typename Func>
void AVLTree<K, V>::inOrderHelper(AVLNode<K, V> *node, Func visit) const
{
    if (node == nullptr)
        return;

    inOrderHelper(node->left, visit);
    visit(node->key, node->data);
    inOrderHelper(node->right, visit);
}

template <typename K, typename V>
template <typename Func>
void AVLTree<K, V>::traverseInOrder(Func visit) const
{
    inOrderHelper(root, visit);
}