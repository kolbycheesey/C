#ifndef LSM_TREE_TPP
#define LSM_TREE_TPP

#include "lsm_tree.h"
#include <filesystem>
#include <thread>

template <typename Key, typename Value>
LSMTree<Key, Value>::LSMTree(const std::string& directory, size_t memTableSizeMB)
    : dataDirectory(directory), memTableSizeBytes(memTableSizeMB * 1024 * 1024), stopRequested(false) {
    
    // Create data directory if it doesn't exist
    std::filesystem::create_directories(directory);
    
    // Initialize components
    allocator = std::make_unique<MemoryAllocator>();
    mmapManager = std::make_unique<MMapManager>();
    
    // Create the active memtable
    activeMemTable = createMemTable();
    
    // Initialize compaction manager
    compactionManager = std::make_unique<CompactionManager<Key, Value>>(
        mmapManager.get(), dataDirectory);
    
    // Start background flush thread
    flushThread = std::thread(&LSMTree::flushThreadFunc, this);
}

template <typename Key, typename Value>
LSMTree<Key, Value>::~LSMTree() {
    // Signal flush thread to stop and wait for it
    stopRequested = true;
    flushCV.notify_all();
    
    if (flushThread.joinable()) {
        flushThread.join();
    }
    
    // Flush any remaining memtables
    flush();
}

template <typename Key, typename Value>
std::unique_ptr<MemTable<Key, Value>> LSMTree<Key, Value>::createMemTable() {
    return std::make_unique<MemTable<Key, Value>>(memTableSizeBytes, allocator.get());
}

template <typename Key, typename Value>
void LSMTree<Key, Value>::flushThreadFunc() {
    while (!stopRequested) {
        std::unique_ptr<MemTable<Key, Value>> tableToFlush;
        
        {
            std::unique_lock<std::mutex> lock(mutex);
            
            // Wait until we have an immutable memtable to flush or stop is requested
            flushCV.wait(lock, [this] {
                return stopRequested || !immutableMemTables.empty();
            });
            
            if (stopRequested && immutableMemTables.empty()) {
                break;
            }
            
            if (!immutableMemTables.empty()) {
                // Take the oldest immutable memtable for flushing
                tableToFlush = std::move(immutableMemTables.front());
                immutableMemTables.erase(immutableMemTables.begin());
            }
        }
        
        if (tableToFlush) {
            flushMemTable(tableToFlush.get());
        }
    }
}

template <typename Key, typename Value>
void LSMTree<Key, Value>::flushMemTable(MemTable<Key, Value>* memtable) {
    try {
        // Create an SSTable from the memtable
        auto sstable = SSTable<Key, Value>::createFromMemTable(
            *memtable, mmapManager.get(), dataDirectory, 0);
        
        // Add the SSTable to the compaction manager
        compactionManager->addTable(std::move(sstable));
    }
    catch (const std::exception& ex) {
        // In a production system, we would log the error and handle it appropriately
        std::cerr << "Error flushing memtable: " << ex.what() << std::endl;
    }
}

template <typename Key, typename Value>
bool LSMTree<Key, Value>::put(const Key& key, const Value& value) {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Try to insert into the active memtable
    if (!activeMemTable->put(key, value)) {
        // If it's full, make it immutable
        activeMemTable->makeImmutable();
        immutableMemTables.push_back(std::move(activeMemTable));
        
        // Create a new active memtable
        activeMemTable = createMemTable();
        
        // Notify flush thread
        flushCV.notify_one();
        
        // Try again with the new memtable
        return activeMemTable->put(key, value);
    }
    
    return true;
}

template <typename Key, typename Value>
bool LSMTree<Key, Value>::remove(const Key& key) {
    std::unique_lock<std::mutex> lock(mutex);
    
    // In a real implementation, we would insert a tombstone marker
    // For simplicity, we'll just remove from active memtable
    return activeMemTable->remove(key);
}

template <typename Key, typename Value>
std::optional<Value> LSMTree<Key, Value>::get(const Key& key) {
    // First check active memtable
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        Value value;
        if (activeMemTable->get(key, value)) {
            return value;
        }
        
        // Check immutable memtables (newest to oldest)
        for (auto it = immutableMemTables.rbegin(); it != immutableMemTables.rend(); ++it) {
            if ((*it)->get(key, value)) {
                return value;
            }
        }
    }
    
    // Check SSTables using compaction manager
    auto tables = compactionManager->getTablesForKey(key);
    
    // Check tables from newest to oldest
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        auto result = (*it)->get(key);
        if (result) {
            return *result;
        }
    }
    
    // Key not found
    return std::nullopt;
}

template <typename Key, typename Value>
std::vector<std::pair<Key, Value>> LSMTree<Key, Value>::range(
    const Key& startKey, const Key& endKey) {
    
    std::vector<std::pair<Key, Value>> result;
    std::map<Key, Value> mergedResult; // For deduplication
    
    // First collect from active memtable
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        auto activeResults = activeMemTable->range(startKey, endKey);
        for (const auto& [key, value] : activeResults) {
            mergedResult[key] = value;
        }
        
        // Collect from immutable memtables (newest to oldest)
        for (auto it = immutableMemTables.rbegin(); it != immutableMemTables.rend(); ++it) {
            auto tableResults = (*it)->range(startKey, endKey);
            for (const auto& [key, value] : tableResults) {
                // Only insert if not already present (newer value takes precedence)
                if (mergedResult.find(key) == mergedResult.end()) {
                    mergedResult[key] = value;
                }
            }
        }
    }
    
    // Collect from SSTables
    auto tables = compactionManager->getTablesForRange(startKey, endKey);
    
    // Process tables from newest to oldest
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        auto tableResults = (*it)->range(startKey, endKey);
        for (const auto& [key, value] : tableResults) {
            // Only insert if not already present (newer value takes precedence)
            if (mergedResult.find(key) == mergedResult.end()) {
                mergedResult[key] = value;
            }
        }
    }
    
    // Convert map back to vector
    result.reserve(mergedResult.size());
    for (const auto& [key, value] : mergedResult) {
        if (key >= startKey && key <= endKey) {
            result.emplace_back(key, value);
        }
    }
    
    return result;
}

template <typename Key, typename Value>
void LSMTree<Key, Value>::flush() {
    // Make active memtable immutable and flush all immutable memtables
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        // Make active memtable immutable if it's not empty
        if (activeMemTable->size() > 0) {
            activeMemTable->makeImmutable();
            immutableMemTables.push_back(std::move(activeMemTable));
            activeMemTable = createMemTable();
            flushCV.notify_one();
        }
    }
    
    // Wait for all memtables to be flushed
    bool allFlushed = false;
    while (!allFlushed) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        std::unique_lock<std::mutex> lock(mutex);
        allFlushed = immutableMemTables.empty();
    }
}

template <typename Key, typename Value>
void LSMTree<Key, Value>::compact(int level, bool majorCompaction) {
    compactionManager->scheduleCompaction(level, majorCompaction);
    
    // Optionally wait for compaction to complete
    if (majorCompaction) {
        compactionManager->waitForCompactions();
    }
}

template <typename Key, typename Value>
size_t LSMTree<Key, Value>::getMemTableSize() const {
    std::unique_lock<std::mutex> lock(mutex);
    return activeMemTable->size();
}

template <typename Key, typename Value>
size_t LSMTree<Key, Value>::getImmutableMemTableCount() const {
    std::unique_lock<std::mutex> lock(mutex);
    return immutableMemTables.size();
}

template <typename Key, typename Value>
std::vector<size_t> LSMTree<Key, Value>::getSSTableCountsByLevel() const {
    std::vector<size_t> counts;
    size_t levelCount = compactionManager->getLevelCount();
    counts.reserve(levelCount);
    
    for (size_t level = 0; level < levelCount; ++level) {
        counts.push_back(compactionManager->getTableCount(level));
    }
    
    return counts;
}

template <typename Key, typename Value>
void LSMTree<Key, Value>::clear() {
    std::cout << "Clearing LSM tree resources..." << std::endl;
    
    // Stop background flush thread if it's still running
    stopRequested = true;
    flushCV.notify_all();
    
    if (flushThread.joinable()) {
        std::cout << "  Joining LSM flush thread..." << std::endl;
        flushThread.join();
        std::cout << "  LSM flush thread joined." << std::endl;
    }

    std::cout << "  Clearing active memtable..." << std::endl;
    // Clear active memtable
    if (activeMemTable) {
        activeMemTable->clear();
    }
    
    std::cout << "  Clearing immutable memtables (" << immutableMemTables.size() << " tables)..." << std::endl;
    // Clear immutable memtables
    for (auto& table : immutableMemTables) {
        if (table) {
            table->clear();
        }
    }
    immutableMemTables.clear();
    
    std::cout << "  Clearing compaction manager..." << std::endl;
    // Clear compaction manager
    if (compactionManager) {
        compactionManager->shutdown();
    }
    
    std::cout << "  Clearing mmap manager..." << std::endl;
    // Close any open memory-mapped files
    if (mmapManager) {
        mmapManager->closeAll();
    }
    
    std::cout << "LSM tree resources cleared." << std::endl;
}

#endif // LSM_TREE_TPP