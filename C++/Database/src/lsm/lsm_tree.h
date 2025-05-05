#ifndef LSM_TREE_H
#define LSM_TREE_H

#include "memtable.h"
#include "sstable.h"
#include "compaction.h"
#include "../storage/mmap_manager.h"
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <optional>
#include <atomic>

/**
 * LSMTree - Log-Structured Merge Tree implementation
 * 
 * This class provides a complete LSM-Tree storage engine with:
 * - Fast write path through MemTable
 * - Persistent storage via SSTables
 * - Background compaction for performance maintenance
 * - Write-ahead logging for durability
 */
template <typename Key, typename Value>
class LSMTree {
private:
    // Current active memtable for writes
    std::unique_ptr<MemTable<Key, Value>> activeMemTable;
    
    // Immutable memtables waiting to be flushed to disk
    std::vector<std::unique_ptr<MemTable<Key, Value>>> immutableMemTables;
    
    // Compaction manager for SSTables
    std::unique_ptr<CompactionManager<Key, Value>> compactionManager;
    
    // Memory-mapped file manager
    std::unique_ptr<MMapManager> mmapManager;
    
    // Custom memory allocator
    std::unique_ptr<MemoryAllocator> allocator;
    
    // Settings
    std::string dataDirectory;
    size_t memTableSizeBytes;
    
    // Mutex for protecting memtable operations
    std::mutex mutex;
    
    // Background flushing thread state
    std::thread flushThread;
    std::condition_variable flushCV;
    std::atomic<bool> stopRequested;
    
    // Background flushing thread function
    void flushThreadFunc();
    
    // Create a new memtable
    std::unique_ptr<MemTable<Key, Value>> createMemTable();
    
    // Flush an immutable memtable to disk
    void flushMemTable(MemTable<Key, Value>* memtable);

public:
    LSMTree(const std::string& directory, size_t memTableSizeMB = 64);
    ~LSMTree();
    
    // Write operations
    bool put(const Key& key, const Value& value);
    bool remove(const Key& key);
    
    // Read operations
    std::optional<Value> get(const Key& key);
    std::vector<std::pair<Key, Value>> range(const Key& startKey, const Key& endKey);
    
    // Administrative operations
    void flush();
    void compact(int level = 0, bool majorCompaction = true);
    
    // Statistics
    size_t getMemTableSize() const;
    size_t getImmutableMemTableCount() const;
    std::vector<size_t> getSSTableCountsByLevel() const;
};

#include "lsm_tree.tpp"

#endif // LSM_TREE_H