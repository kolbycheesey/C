#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include "../memory/memory_allocator.h"

/**
 * MemTable - In-memory sorted structure that buffers recent writes
 * 
 * The MemTable provides fast write performance by storing key-value pairs
 * in memory before flushing to disk. It maintains keys in sorted order
 * for efficient lookups and range queries.
 */
template <typename Key, typename Value>
class MemTable {
private:
    using KeyValueMap = std::map<Key, Value>;
    KeyValueMap data;
    mutable std::mutex mutex;  // Mark mutex as mutable to allow locking in const methods
    size_t memoryUsage;
    const size_t memoryLimit;
    std::atomic<bool> immutable;

    // Optional: Custom allocator for better memory management
    MemoryAllocator* allocator;

public:
    MemTable(size_t maxMemoryBytes, MemoryAllocator* alloc = nullptr);
    
    /**
     * Insert a key-value pair into the memtable
     * @return true if successful, false if memtable is immutable or memory limit reached
     */
    bool put(const Key& key, const Value& value);
    
    /**
     * Look up a value by key
     * @return true if found, false otherwise
     */
    bool get(const Key& key, Value& value) const;
    
    /**
     * Delete a key from the memtable (tombstone)
     */
    bool remove(const Key& key);
    
    /**
     * Make this memtable immutable to prepare for flushing to disk
     */
    void makeImmutable();
    
    /**
     * Check if this memtable is immutable
     */
    bool isImmutable() const;
    
    /**
     * Get approximate memory usage
     */
    size_t getMemoryUsage() const;
    
    /**
     * Get number of entries
     */
    size_t size() const;
    
    /**
     * Check if memtable is full (memory usage exceeds limit)
     */
    bool isFull() const;
    
    /**
     * Get iterator to all entries
     */
    auto begin() const { return data.cbegin(); }
    auto end() const { return data.cend(); }
    
    /**
     * Range query - return all key-value pairs in the range [startKey, endKey]
     */
    std::vector<std::pair<Key, Value>> range(const Key& startKey, const Key& endKey) const;
    
    /**
     * Apply a function to each entry in the memtable
     */
    void forEach(const std::function<void(const Key&, const Value&)>& func) const;
    
    /**
     * Clear all entries from the memtable
     */
    void clear();
};

#include "memtable.tpp"

#endif // MEMTABLE_H