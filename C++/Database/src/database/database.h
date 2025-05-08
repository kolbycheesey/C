#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include "../index/bplus_tree.h"
#include "../lsm/lsm_tree.h"
#include "../query/query_processor.h"
#include "../memory/memory_manager.h"
#include "../storage/storage_engine.h"

class Database {
private:
    std::unique_ptr<StorageEngine> storage;
    std::unique_ptr<MemoryManager> memoryManager;
    std::unique_ptr<QueryProcessor> queryProcessor;
    std::string name;
    
    // B+ Tree index for read-optimized access
    BPlusTree<int, std::string, 128> indexTree;
    
    // LSM Tree for write-optimized storage
    LSMTree<int, std::string> lsmTree;
    
    // Synchronization for hybrid data structure access
    mutable std::mutex accessMutex;
    std::atomic<bool> syncInProgress;
    
    // Background thread for syncing LSM Tree to B+Tree
    std::thread syncThread;
    std::atomic<bool> stopSync;
    
    // Sync data from LSM Tree to B+Tree periodically
    void syncDataStructures();

public:
    Database(const std::string& dbName);
    ~Database();
    
    // Write operations (LSM Tree only)
    bool put(int key, const std::string& value);
    bool remove(int key);
    
    // Read operations (B+Tree only, with fallback to LSM)
    bool get(int key, std::string& value) const;
    std::vector<std::pair<int, std::string>> range(int startKey, int endKey) const;
    
    // Force sync between data structures
    void sync();
};

#endif // DATABASE_H