#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include <vector>
#include <algorithm>
#include <memory>
#include <array>

template<typename Key, typename Value, size_t B = 128>
class BPlusTree {
private:
    // Forward declarations
    struct InnerNode;
    struct LeafNode;
    struct Node;

    // Base Node structure
    struct Node {
        bool isLeaf;
        size_t size;

        Node(bool leaf) : isLeaf(leaf), size(0) {}
        virtual ~Node() = default;
    };

    // Leaf Node containing actual data
    struct LeafNode : public Node {
        // Cache-friendly arrays for keys and values
        std::array<Key, B> keys;
        std::array<Value, B> values;
        LeafNode* nextLeaf; // For range queries

        LeafNode() : Node(true), nextLeaf(nullptr) {}

        // Binary search for key position
        size_t findPos(const Key& key) const {
            return std::lower_bound(
                keys.begin(), 
                keys.begin() + this->size, 
                key
            ) - keys.begin();
        }
    };

    // Inner Node for navigation
    struct InnerNode : public Node {
        std::array<Key, B> keys;
        std::array<Node*, B + 1> children;

        InnerNode() : Node(false) {}
        
        ~InnerNode() override {
            for (size_t i = 0; i <= this->size; ++i) {
                delete children[i];
            }
        }

        // Binary search for child position
        size_t findChildPos(const Key& key) const {
            auto it = std::upper_bound(
                keys.begin(), 
                keys.begin() + this->size, 
                key
            );
            return it - keys.begin();
        }
    };

    Node* root;
    size_t height;
    size_t count;

    // Insert into leaf node
    std::pair<Key, Node*> insertIntoLeaf(LeafNode* leaf, const Key& key, const Value& value);

    // Insert into inner node after a child split
    std::pair<Key, Node*> insertIntoInner(InnerNode* inner, size_t childPos, 
                                         const Key& key, Node* rightNode);

    // Helper function for recursive insertion
    std::pair<Key, Node*> insertHelper(Node* node, const Key& key, const Value& value);

public:
    BPlusTree();
    ~BPlusTree();

    // Insert a key-value pair
    void insert(const Key& key, const Value& value);

    // Find a value by key
    Value* find(const Key& key);
    
    // Range query - highly optimized for cache-friendly access
    std::vector<std::pair<Key, Value>> range(const Key& start, const Key& end);
    
    // Count of elements
    size_t size() const;
    
    // Tree height
    size_t getHeight() const;
};

// Include template implementation
#include "bplus_tree.tpp"

#endif // BPLUS_TREE_H