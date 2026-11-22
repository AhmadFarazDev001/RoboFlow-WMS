//
//  AVLTree.ipp
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 21/11/2026.
//

#ifndef AVLTREE_IPP
#define AVLTREE_IPP

template <typename K, typename V>
AVLTree<K, V>::AVLTree() : root(nullptr), nodeCount(0) {}

template <typename K, typename V>
AVLTree<K, V>::~AVLTree() { clear(); }

template <typename K, typename V>
void AVLTree<K, V>::clear() {
    clearTree(root);
    root = nullptr;
    nodeCount = 0;
}

template <typename K, typename V>
void AVLTree<K, V>::clearTree(AVLNode<K, V>* node) {
    if (!node) return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

template <typename K, typename V>
bool AVLTree<K, V>::insert(const K& key, const V& value) {
    bool inserted = false;
    root = insertNode(root, key, value, inserted);
    if (inserted) ++nodeCount;
    return inserted;
}

template <typename K, typename V>
AVLNode<K, V>* AVLTree<K, V>::insertNode(AVLNode<K, V>* node, const K& key, const V& value, bool& inserted) {
    if (!node) {
        inserted = true;
        return new AVLNode<K, V>(key, value);
    }
    if (key < node->key) node->left = insertNode(node->left, key, value, inserted);
    else if (key > node->key) node->right = insertNode(node->right, key, value, inserted);
    return node;
}

template <typename K, typename V>
V* AVLTree<K, V>::search(const K& key) {
    AVLNode<K, V>* found = searchNode(root, key);
    return found ? &(found->data) : nullptr;
}

template <typename K, typename V>
const V* AVLTree<K, V>::search(const K& key) const {
    AVLNode<K, V>* found = searchNode(root, key);
    return found ? &(found->data) : nullptr;
}

template <typename K, typename V>
AVLNode<K, V>* AVLTree<K, V>::searchNode(AVLNode<K, V>* node, const K& key) const {
    if (!node) return nullptr;
    if (key == node->key) return node;
    if (key < node->key) return searchNode(node->left, key);
    return searchNode(node->right, key);
}

template <typename K, typename V>
int AVLTree<K, V>::getSize() const { return nodeCount; }

template <typename K, typename V>
bool AVLTree<K, V>::isEmpty() const { return nodeCount == 0; }

#endif
