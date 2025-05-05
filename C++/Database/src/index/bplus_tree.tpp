#ifndef BPLUS_TREE_TPP
#define BPLUS_TREE_TPP

#include "bplus_tree.h"

template<typename Key, typename Value, size_t B>
BPlusTree<Key, Value, B>::BPlusTree() : root(new LeafNode()), height(1), count(0) {}

template<typename Key, typename Value, size_t B>
BPlusTree<Key, Value, B>::~BPlusTree() {
    delete root;
}

template<typename Key, typename Value, size_t B>
std::pair<Key, typename BPlusTree<Key, Value, B>::Node*> 
BPlusTree<Key, Value, B>::insertIntoLeaf(LeafNode* leaf, const Key& key, const Value& value) {
    size_t pos = leaf->findPos(key);
    
    // If key exists, update value
    if (pos < leaf->size && leaf->keys[pos] == key) {
        leaf->values[pos] = value;
        return {Key(), nullptr}; // No split needed
    }
    
    // If leaf is full, need to split
    if (leaf->size >= B) {
        // Create new leaf
        LeafNode* newLeaf = new LeafNode();
        
        // Split point
        size_t mid = B / 2;
        
        // Copy second half to new leaf
        for (size_t i = mid; i < leaf->size; ++i) {
            newLeaf->keys[i - mid] = leaf->keys[i];
            newLeaf->values[i - mid] = leaf->values[i];
            ++newLeaf->size;
        }
        
        // Adjust size of original leaf
        leaf->size = mid;
        
        // Link leaves
        newLeaf->nextLeaf = leaf->nextLeaf;
        leaf->nextLeaf = newLeaf;
        
        // Determine where to insert the new key
        if (pos < mid) {
            // Insert into original leaf
            for (size_t i = leaf->size; i > pos; --i) {
                leaf->keys[i] = leaf->keys[i - 1];
                leaf->values[i] = leaf->values[i - 1];
            }
            leaf->keys[pos] = key;
            leaf->values[pos] = value;
            ++leaf->size;
        } else {
            // Insert into new leaf
            pos = pos - mid;
            for (size_t i = newLeaf->size; i > pos; --i) {
                newLeaf->keys[i] = newLeaf->keys[i - 1];
                newLeaf->values[i] = newLeaf->values[i - 1];
            }
            newLeaf->keys[pos] = key;
            newLeaf->values[pos] = value;
            ++newLeaf->size;
        }
        
        // Return the split info
        return {newLeaf->keys[0], newLeaf};
    }
    
    // No split needed, just insert
    for (size_t i = leaf->size; i > pos; --i) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->values[i] = leaf->values[i - 1];
    }
    leaf->keys[pos] = key;
    leaf->values[pos] = value;
    ++leaf->size;
    
    return {Key(), nullptr}; // No split needed
}

template<typename Key, typename Value, size_t B>
std::pair<Key, typename BPlusTree<Key, Value, B>::Node*> 
BPlusTree<Key, Value, B>::insertIntoInner(InnerNode* inner, size_t childPos, 
                                       const Key& key, Node* rightNode) {
    // If inner node is full, need to split
    if (inner->size >= B) {
        // Create new inner node
        InnerNode* newInner = new InnerNode();
        
        // Split point
        size_t mid = B / 2;
        
        // Copy second half keys to new node
        for (size_t i = mid + 1; i < inner->size; ++i) {
            newInner->keys[i - mid - 1] = inner->keys[i];
            ++newInner->size;
        }
        
        // Copy second half children to new node
        for (size_t i = mid + 1; i <= inner->size; ++i) {
            newInner->children[i - mid - 1] = inner->children[i];
            inner->children[i] = nullptr;
        }
        
        Key splitKey = inner->keys[mid];
        inner->size = mid;
        
        // Determine where to insert the new key and child
        if (childPos <= mid) {
            // Insert into original inner node
            for (size_t i = inner->size; i > childPos; --i) {
                inner->keys[i] = inner->keys[i - 1];
                inner->children[i + 1] = inner->children[i];
            }
            inner->keys[childPos] = key;
            inner->children[childPos + 1] = rightNode;
            ++inner->size;
        } else {
            // Insert into new inner node
            childPos = childPos - mid - 1;
            for (size_t i = newInner->size; i > childPos; --i) {
                newInner->keys[i] = newInner->keys[i - 1];
                newInner->children[i + 1] = newInner->children[i];
            }
            newInner->keys[childPos] = key;
            newInner->children[childPos + 1] = rightNode;
            ++newInner->size;
        }
        
        // Return the split info
        return {splitKey, newInner};
    }
    
    // No split needed, just insert
    for (size_t i = inner->size; i > childPos; --i) {
        inner->keys[i] = inner->keys[i - 1];
        inner->children[i + 1] = inner->children[i];
    }
    inner->keys[childPos] = key;
    inner->children[childPos + 1] = rightNode;
    ++inner->size;
    
    return {Key(), nullptr}; // No split needed
}

template<typename Key, typename Value, size_t B>
std::pair<Key, typename BPlusTree<Key, Value, B>::Node*> 
BPlusTree<Key, Value, B>::insertHelper(Node* node, const Key& key, const Value& value) {
    if (node->isLeaf) {
        return insertIntoLeaf(static_cast<LeafNode*>(node), key, value);
    } else {
        InnerNode* inner = static_cast<InnerNode*>(node);
        size_t childPos = inner->findChildPos(key);
        
        auto [splitKey, splitNode] = insertHelper(inner->children[childPos], key, value);
        
        if (!splitNode) {
            return {Key(), nullptr}; // No split occurred
        }
        
        return insertIntoInner(inner, childPos, splitKey, splitNode);
    }
}

template<typename Key, typename Value, size_t B>
void BPlusTree<Key, Value, B>::insert(const Key& key, const Value& value) {
    auto [splitKey, splitNode] = insertHelper(root, key, value);
    
    // If root was split, create new root
    if (splitNode) {
        InnerNode* newRoot = new InnerNode();
        newRoot->keys[0] = splitKey;
        newRoot->children[0] = root;
        newRoot->children[1] = splitNode;
        newRoot->size = 1;
        
        root = newRoot;
        ++height;
    }
    
    ++count;
}

template<typename Key, typename Value, size_t B>
Value* BPlusTree<Key, Value, B>::find(const Key& key) {
    Node* node = root;
    
    // Traverse to leaf
    while (!node->isLeaf) {
        InnerNode* inner = static_cast<InnerNode*>(node);
        size_t pos = inner->findChildPos(key);
        node = inner->children[pos];
    }
    
    // Search in leaf
    LeafNode* leaf = static_cast<LeafNode*>(node);
    size_t pos = leaf->findPos(key);
    
    if (pos < leaf->size && leaf->keys[pos] == key) {
        return &leaf->values[pos];
    }
    
    return nullptr; // Not found
}

template<typename Key, typename Value, size_t B>
std::vector<std::pair<Key, Value>> BPlusTree<Key, Value, B>::range(const Key& start, const Key& end) {
    std::vector<std::pair<Key, Value>> result;
    
    // Find the leaf containing the start key
    Node* node = root;
    while (!node->isLeaf) {
        InnerNode* inner = static_cast<InnerNode*>(node);
        size_t pos = inner->findChildPos(start);
        node = inner->children[pos];
    }
    
    // Traverse leaves and collect results
    LeafNode* leaf = static_cast<LeafNode*>(node);
    while (leaf) {
        size_t pos = 0;
        
        // Find position of first key >= start
        if (leaf == node) { // Only for the first leaf
            pos = leaf->findPos(start);
        }
        
        // Collect keys until we hit the end key
        while (pos < leaf->size && leaf->keys[pos] <= end) {
            result.emplace_back(leaf->keys[pos], leaf->values[pos]);
            ++pos;
        }
        
        // If we've processed all keys up to end, we're done
        if (pos < leaf->size && leaf->keys[pos] > end) {
            break;
        }
        
        leaf = leaf->nextLeaf;
    }
    
    return result;
}

template<typename Key, typename Value, size_t B>
size_t BPlusTree<Key, Value, B>::size() const {
    return count;
}

template<typename Key, typename Value, size_t B>
size_t BPlusTree<Key, Value, B>::getHeight() const {
    return height;
}

#endif // BPLUS_TREE_TPP