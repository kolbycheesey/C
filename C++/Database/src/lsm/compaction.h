#ifndef COMPACTION_H
#define COMPACTION_H

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <functional>
#include "sstable.h"

/**
 * CompactionManager - Handles the process of merging SSTables in the LSM-Tree
 * 
 * This class manages the background compaction process that merges multiple
 * SSTables at each level into fewer, larger SSTables in the next level.
 * This is crucial for maintaining read performance over time.
 */
template <typename Key, typename Value>
class CompactionManager {
public:
    using SSTablePtr = std::unique_ptr<SSTable<Key, Value>>;
    using SSTableList = std::vector<SSTablePtr>;

private:
    // Storage engine for accessing file system
    MMapManager* mmapManager;
    
    // Data directory for SSTables
    std::string dataDirectory;
    
    // Maximum number of SSTables per level before triggering compaction
    std::vector<size_t> maxTablesPerLevel;
    
    // The actual SSTables organized by level
    std::vector<SSTableList> levels;
    
    // Mutex for protecting levels
    std::mutex mutex;
    
    // Background compaction thread
    std::thread compactionThread;
    
    // For signaling the compaction thread
    std::condition_variable compactionCV;
    
    // Queue of compaction jobs
    std::queue<std::pair<int, bool>> compactionQueue; // level, major compaction flag
    
    // Flag for stopping the background thread
    std::atomic<bool> stopRequested;
    
    // Run the background compaction thread
    void compactionThreadFunc();
    
    // Check if compaction is needed for a level
    bool isCompactionNeeded(int level) const;
    
    // Perform compaction for a level
    void compactLevel(int level, bool majorCompaction);
    
    // Merge multiple SSTables into one
    SSTablePtr mergeTables(const std::vector<SSTablePtr*>& tables);

public:
    CompactionManager(MMapManager* mmapManager, const std::string& dataDirectory);
    
    ~CompactionManager();
    
    // Add a new SSTable to level 0
    void addTable(SSTablePtr table);
    
    // Schedule compaction for a level
    void scheduleCompaction(int level, bool majorCompaction = false);
    
    // Get all SSTables that might contain a key
    std::vector<SSTable<Key, Value>*> getTablesForKey(const Key& key);
    
    // Get all SSTables for a range query
    std::vector<SSTable<Key, Value>*> getTablesForRange(const Key& startKey, const Key& endKey);
    
    // Get number of levels
    size_t getLevelCount() const;
    
    // Get number of tables at a level
    size_t getTableCount(int level) const;
    
    // Wait for all compactions to complete
    void waitForCompactions();
};

#include "compaction.tpp"

#endif // COMPACTION_H