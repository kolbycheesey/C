#ifndef SSTABLE_H
#define SSTABLE_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <functional>
#include <optional>
#include "../storage/mmap_manager.h"

// Forward declaration
template <typename Key, typename Value>
class MemTable;

/**
 * SSTable - Sorted String Table for on-disk storage in the LSM-Tree
 * 
 * This class represents an immutable, sorted table of key-value pairs stored on disk.
 * It includes index blocks for fast lookups and supports Bloom filters to quickly
 * determine if a key might be present.
 */
template <typename Key, typename Value>
class SSTable {
public:
    // Structure to represent a key-value entry in the index
    struct IndexEntry {
        Key key;
        uint64_t offset;
        uint32_t size;
    };

    // Structure to represent an SSTable metadata
    struct Metadata {
        uint32_t keyCount;
        uint64_t dataSize;
        uint64_t indexOffset;
        uint32_t level;
        std::string filePath;
        Key minKey;
        Key maxKey;
    };

private:
    // Memory-mapped file manager for I/O operations
    MMapManager* mmapManager;
    
    // Metadata about this SSTable
    Metadata metadata;
    
    // In-memory index for fast lookups
    std::vector<IndexEntry> index;
    
    // Cached data pointer from memory-mapped file
    void* dataPtr;
    
    // Bloom filter for fast negative lookups
    // In a production implementation, we'd have a proper Bloom filter class here
    // For simplicity, we'll skip the actual implementation details
    
    // Load index from file
    void loadIndex();
    
    // Binary search in index
    int64_t findIndexEntry(const Key& key) const;
    
    // Read value from data file at offset
    Value readValueAt(uint64_t offset, uint32_t size) const;

public:
    // Create a new SSTable from a MemTable
    static std::unique_ptr<SSTable<Key, Value>> createFromMemTable(
        const MemTable<Key, Value>& memTable, 
        MMapManager* mmapManager,
        const std::string& directory,
        uint32_t level);
    
    // Open an existing SSTable
    SSTable(MMapManager* mmapManager, const std::string& filePath);
    
    // Destructor to cleanup resources
    ~SSTable();
    
    // Check if key potentially exists (Bloom filter check)
    bool mayContain(const Key& key) const;
    
    // Get value for a key
    std::optional<Value> get(const Key& key) const;
    
    // Range query from start key to end key
    std::vector<std::pair<Key, Value>> range(const Key& startKey, const Key& endKey) const;
    
    // Get metadata
    const Metadata& getMetadata() const;
    
    // Get a specific index entry
    const IndexEntry& getIndexEntry(size_t pos) const;
    
    // Get number of index entries
    size_t getIndexSize() const;
    
    // Get file path
    const std::string& getFilePath() const;
    
    // Apply a function to each entry in the table
    void forEach(const std::function<void(const Key&, const Value&)>& func) const;
};

#include "sstable.tpp"

#endif // SSTABLE_H