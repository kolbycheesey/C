#ifndef COMPACTION_TPP
#define COMPACTION_TPP

#include "compaction.h"
#include "memtable.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <set>

template <typename Key, typename Value>
CompactionManager<Key, Value>::CompactionManager(
    MMapManager* mmapManager, const std::string& dataDirectory)
    : mmapManager(mmapManager), dataDirectory(dataDirectory), stopRequested(false) {
    
    // Initialize level configuration
    // Level 0: 4 tables
    // Level 1: 10 tables
    // Level 2: 100 tables
    // ... each level is 10x the size of the previous
    maxTablesPerLevel = {4, 10, 100, 1000};
    
    // Initialize levels
    levels.resize(maxTablesPerLevel.size());
    
    // Scan existing SSTables in the data directory and load them
    if (std::filesystem::exists(dataDirectory)) {
        for (const auto& entry : std::filesystem::directory_iterator(dataDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".db") {
                std::string filename = entry.path().filename().string();
                
                // Parse the level from filename (format: sstable_L{level}_{timestamp}.db)
                if (filename.find("sstable_L") == 0) {
                    size_t levelPos = filename.find('_', 9) + 1;
                    int level = std::stoi(filename.substr(9, levelPos - 10));
                    
                    if (level < static_cast<int>(levels.size())) {
                        try {
                            auto table = std::make_unique<SSTable<Key, Value>>(
                                mmapManager, entry.path().string());
                            levels[level].push_back(std::move(table));
                        } catch (const std::exception& ex) {
                            std::cerr << "Failed to load SSTable: " << entry.path().string()
                                      << " Error: " << ex.what() << std::endl;
                        }
                    }
                }
            }
        }
    } else {
        // Create directory if it doesn't exist
        std::filesystem::create_directories(dataDirectory);
    }
    
    // Start compaction thread
    compactionThread = std::thread(&CompactionManager::compactionThreadFunc, this);
}

template <typename Key, typename Value>
CompactionManager<Key, Value>::~CompactionManager() {
    // Stop compaction thread
    {
        std::unique_lock<std::mutex> lock(mutex);
        stopRequested = true;
        compactionCV.notify_all();
    }
    
    if (compactionThread.joinable()) {
        compactionThread.join();
    }
}

template <typename Key, typename Value>
void CompactionManager<Key, Value>::compactionThreadFunc() {
    while (!stopRequested) {
        int levelToCompact = -1;
        bool majorCompaction = false;
        
        {
            std::unique_lock<std::mutex> lock(mutex);
            
            // Wait for compaction job or stop signal
            compactionCV.wait(lock, [this] {
                return stopRequested || !compactionQueue.empty();
            });
            
            if (stopRequested) {
                break;
            }
            
            auto job = compactionQueue.front();
            compactionQueue.pop();
            levelToCompact = job.first;
            majorCompaction = job.second;
        }
        
        if (levelToCompact >= 0) {
            compactLevel(levelToCompact, majorCompaction);
        }
    }
}

template <typename Key, typename Value>
bool CompactionManager<Key, Value>::isCompactionNeeded(int level) const {
    return level < static_cast<int>(levels.size()) && 
           levels[level].size() > maxTablesPerLevel[level];
}

template <typename Key, typename Value>
void CompactionManager<Key, Value>::compactLevel(int level, bool majorCompaction) {
    if (level >= static_cast<int>(levels.size()) - 1) {
        // Last level, nothing to compact into
        return;
    }
    
    std::vector<SSTablePtr> tablesToCompact;
    std::vector<SSTablePtr*> tablePointers;
    
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        if (levels[level].empty()) {
            return;
        }
        
        // For major compaction, take all tables from the level
        // For minor compaction, take some tables based on size, overlap, etc.
        if (majorCompaction || isCompactionNeeded(level)) {
            tablesToCompact.reserve(levels[level].size());
            
            // Move all tables from this level for compaction
            for (auto& table : levels[level]) {
                tablesToCompact.push_back(std::move(table));
            }
            
            // Clear the level
            levels[level].clear();
        } else {
            // For minor compaction, just take the oldest few tables
            size_t tablesToTake = std::min(levels[level].size(), size_t(2));
            tablesToCompact.reserve(tablesToTake);
            
            for (size_t i = 0; i < tablesToTake; ++i) {
                tablesToCompact.push_back(std::move(levels[level][i]));
            }
            
            // Remove the taken tables
            levels[level].erase(levels[level].begin(), levels[level].begin() + tablesToTake);
        }
        
        // If we're compacting level > 0, also include overlapping tables from the next level
        if (level > 0 && !levels[level + 1].empty()) {
            // Find key range of tables we're compacting
            Key minKey = tablesToCompact[0]->getMetadata().minKey;
            Key maxKey = tablesToCompact[0]->getMetadata().maxKey;
            
            for (size_t i = 1; i < tablesToCompact.size(); ++i) {
                minKey = std::min(minKey, tablesToCompact[i]->getMetadata().minKey);
                maxKey = std::max(maxKey, tablesToCompact[i]->getMetadata().maxKey);
            }
            
            // Find overlapping tables from the next level
            std::vector<size_t> overlappingIndexes;
            for (size_t i = 0; i < levels[level + 1].size(); ++i) {
                const auto& table = levels[level + 1][i];
                if (!(table->getMetadata().maxKey < minKey || 
                      table->getMetadata().minKey > maxKey)) {
                    overlappingIndexes.push_back(i);
                }
            }
            
            // Add overlapping tables to compact list
            for (auto it = overlappingIndexes.rbegin(); it != overlappingIndexes.rend(); ++it) {
                tablesToCompact.push_back(std::move(levels[level + 1][*it]));
            }
            
            // Remove overlapping tables from next level
            for (auto it = overlappingIndexes.rbegin(); it != overlappingIndexes.rend(); ++it) {
                levels[level + 1].erase(levels[level + 1].begin() + *it);
            }
        }
    }
    
    // No tables to compact
    if (tablesToCompact.empty()) {
        return;
    }
    
    // Prepare pointers for merge
    for (auto& table : tablesToCompact) {
        tablePointers.push_back(&table);
    }
    
    // Merge tables
    SSTablePtr mergedTable = mergeTables(tablePointers);
    
    if (mergedTable) {
        // Add merged table to the next level
        std::unique_lock<std::mutex> lock(mutex);
        levels[level + 1].push_back(std::move(mergedTable));
        
        // Check if next level needs compaction
        if (isCompactionNeeded(level + 1)) {
            scheduleCompaction(level + 1);
        }
    }
}

template <typename Key, typename Value>
typename CompactionManager<Key, Value>::SSTablePtr 
CompactionManager<Key, Value>::mergeTables(const std::vector<SSTablePtr*>& tables) {
    if (tables.empty()) {
        return nullptr;
    }
    
    // Create a temporary in-memory store for merging
    // In a real implementation, we'd use a direct streaming approach
    // to avoid loading everything into memory
    MemTable<Key, Value> tempMemTable(1024 * 1024 * 1024); // 1GB limit
    
    // Use a custom comparator to merge entries from multiple tables
    struct KeyValuePair {
        Key key;
        Value value;
        size_t tableIndex;
        
        bool operator<(const KeyValuePair& other) const {
            return key < other.key;
        }
    };
    
    // Collect all entries from all tables
    // In a real implementation, we'd use a merge iterator instead
    std::set<KeyValuePair> allEntries;
    
    for (size_t i = 0; i < tables.size(); ++i) {
        const auto& table = *tables[i];
        
        table->forEach([&](const Key& key, const Value& value) {
            allEntries.insert({key, value, i});
        });
    }
    
    // Insert all entries into the memtable
    // If there are duplicate keys, later tables (higher levels) take precedence
    for (const auto& entry : allEntries) {
        tempMemTable.put(entry.key, entry.value);
    }
    
    // Calculate target level (one level down)
    uint32_t targetLevel = (*tables[0])->getMetadata().level + 1;
    
    // Create a new SSTable from the merged data
    return SSTable<Key, Value>::createFromMemTable(
        tempMemTable, mmapManager, dataDirectory, targetLevel);
}

template <typename Key, typename Value>
void CompactionManager<Key, Value>::addTable(SSTablePtr table) {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Add table to level 0
    levels[0].push_back(std::move(table));
    
    // Schedule compaction if needed
    if (isCompactionNeeded(0)) {
        scheduleCompaction(0);
    }
}

template <typename Key, typename Value>
void CompactionManager<Key, Value>::scheduleCompaction(int level, bool majorCompaction) {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Add compaction job to the queue
    compactionQueue.push({level, majorCompaction});
    compactionCV.notify_one();
}

template <typename Key, typename Value>
std::vector<SSTable<Key, Value>*> 
CompactionManager<Key, Value>::getTablesForKey(const Key& key) {
    std::vector<SSTable<Key, Value>*> result;
    std::unique_lock<std::mutex> lock(mutex);
    
    // For level 0, check all tables since they might overlap
    for (const auto& table : levels[0]) {
        if (table->mayContain(key)) {
            result.push_back(table.get());
        }
    }
    
    // For other levels, tables are non-overlapping, so at most one table per level
    for (size_t level = 1; level < levels.size(); ++level) {
        for (const auto& table : levels[level]) {
            if (table->mayContain(key)) {
                result.push_back(table.get());
                break;  // Found the table for this level, no need to check more
            }
        }
    }
    
    return result;
}

template <typename Key, typename Value>
std::vector<SSTable<Key, Value>*> 
CompactionManager<Key, Value>::getTablesForRange(const Key& startKey, const Key& endKey) {
    std::vector<SSTable<Key, Value>*> result;
    std::unique_lock<std::mutex> lock(mutex);
    
    // For level 0, check all tables since they might overlap
    for (const auto& table : levels[0]) {
        if (!(table->getMetadata().maxKey < startKey || 
              table->getMetadata().minKey > endKey)) {
            result.push_back(table.get());
        }
    }
    
    // For other levels, find all tables that overlap with the range
    for (size_t level = 1; level < levels.size(); ++level) {
        for (const auto& table : levels[level]) {
            if (!(table->getMetadata().maxKey < startKey || 
                  table->getMetadata().minKey > endKey)) {
                result.push_back(table.get());
            }
        }
    }
    
    return result;
}

template <typename Key, typename Value>
size_t CompactionManager<Key, Value>::getLevelCount() const {
    return levels.size();
}

template <typename Key, typename Value>
size_t CompactionManager<Key, Value>::getTableCount(int level) const {
    std::unique_lock<std::mutex> lock(mutex);
    
    if (level < 0 || level >= static_cast<int>(levels.size())) {
        return 0;
    }
    
    return levels[level].size();
}

template <typename Key, typename Value>
void CompactionManager<Key, Value>::waitForCompactions() {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Wait until the compaction queue is empty
    compactionCV.wait(lock, [this] {
        return compactionQueue.empty();
    });
}

template <typename Key, typename Value>
void CompactionManager<Key, Value>::shutdown() {
    std::cout << "Shutting down CompactionManager..." << std::endl;
    
    // Stop compaction thread
    {
        std::unique_lock<std::mutex> lock(mutex);
        stopRequested = true;
        compactionQueue = {}; // Clear the queue
        compactionCV.notify_all();
    }
    
    // Wait for compaction thread to finish
    if (compactionThread.joinable()) {
        std::cout << "  Joining compaction thread..." << std::endl;
        compactionThread.join();
        std::cout << "  Compaction thread joined." << std::endl;
    }
    
    // Clean up all SSTables
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::cout << "  Clearing " << levels.size() << " levels of SSTables..." << std::endl;
        
        for (size_t level = 0; level < levels.size(); ++level) {
            size_t tableCount = levels[level].size();
            std::cout << "  Level " << level << ": releasing " << tableCount << " tables..." << std::endl;
            levels[level].clear();
        }
    }
    
    std::cout << "CompactionManager shutdown complete." << std::endl;
}

#endif // COMPACTION_TPP