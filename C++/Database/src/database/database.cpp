#include "database.h"
#include "../storage/mmap_manager.h"
#include "../storage/storage_engine.h"
#include "../query/query_processor.h"
#include <chrono>
#include <limits>

// Database class implementation
Database::Database(const std::string& dbName) 
    : name(dbName), 
      lsmTree("./data/lsm"),
      syncInProgress(false),
      stopSync(false) {
    
    std::cout << "Initializing high-performance database: " << name << std::endl;
    std::cout << "Using hybrid storage approach: LSM Tree for writes, B+Tree for reads" << std::endl;
    
    memoryManager = std::make_unique<MemoryManager>();
    storage = std::make_unique<StorageEngine>("./data");
    queryProcessor = std::make_unique<QueryProcessor>();
    
    // Start background sync thread
    syncThread = std::thread(&Database::syncDataStructures, this);
}

Database::~Database() {
    std::cout << "Database destructor called for " << name << std::endl;
    
    // Signal the sync thread to stop
    stopSync.store(true);
    
    // Wait for sync thread to finish
    if (syncThread.joinable()) {
        std::cout << "Joining sync thread..." << std::endl;
        syncThread.join();
        std::cout << "Sync thread joined successfully" << std::endl;
    }
    
    std::cout << "Destroying components..." << std::endl;
    
    // Explicitly destroy components in a controlled order
    // This helps identify which component might be causing the hang
    
    std::cout << "Clearing LSM tree..." << std::endl;
    lsmTree.clear(); // Add a clear method to your LSM tree if it doesn't exist

    std::cout << "Destroying query processor..." << std::endl;
    queryProcessor.reset();
    
    std::cout << "Destroying memory manager..." << std::endl;
    memoryManager.reset();
    
    std::cout << "Destroying storage engine..." << std::endl;
    storage.reset();
    
    std::cout << "Database " << name << " shutdown completed." << std::endl;
}

bool Database::put(int key, const std::string& value) {
    // Write operations only go to LSM Tree for optimal write performance
    return lsmTree.put(key, value);
}

bool Database::remove(int key) {
    // For removals, we only update the LSM Tree since B+Tree doesn't support removal
    // This is a design limitation - newer values in LSM will overshadow older B+Tree entries
    return lsmTree.remove(key);
}

bool Database::get(int key, std::string& value) const {
    // Read operations primarily from B+Tree for optimal read performance
    {
        std::lock_guard<std::mutex> lock(accessMutex);
        
        // First try B+Tree
        // Need a non-const version for the method call since find() isn't const
        std::string* result = const_cast<BPlusTree<int, std::string, 128>&>(indexTree).find(key);
        if (result) {
            value = *result;
            return true;
        }
    }
    
    // If not in B+Tree, check LSM Tree (this might be newly written data not yet synced)
    // Need to cast away const since LSM Tree's get() isn't marked as const
    auto optionalValue = const_cast<LSMTree<int, std::string>&>(lsmTree).get(key);
    if (optionalValue.has_value()) {
        value = optionalValue.value();
        return true;
    }
    return false;
}

std::vector<std::pair<int, std::string>> Database::range(int startKey, int endKey) const {
    std::vector<std::pair<int, std::string>> results;
    
    // Read operations primarily from B+Tree
    {
        std::lock_guard<std::mutex> lock(accessMutex);
        
        // Get range from B+Tree (need to cast away const because method isn't const)
        results = const_cast<BPlusTree<int, std::string, 128>&>(indexTree).range(startKey, endKey);
    }
    
    // Check LSM Tree for any newer values in the range (need to cast away const)
    auto lsmResults = const_cast<LSMTree<int, std::string>&>(lsmTree).range(startKey, endKey);
    
    // Merge results, preferring LSM Tree values (newer) for the same keys
    for (const auto& pair : lsmResults) {
        // Check if this key already exists in the results from B+Tree
        bool found = false;
        for (auto& existingPair : results) {
            if (existingPair.first == pair.first) {
                // Update with newer value from LSM Tree
                existingPair.second = pair.second;
                found = true;
                break;
            }
        }
        
        // If not found, add it to results
        if (!found) {
            results.push_back(pair);
        }
    }
    
    return results;
}

void Database::sync() {
    std::lock_guard<std::mutex> lock(accessMutex);
    
    // If a sync is already in progress, just return
    if (syncInProgress.load()) {
        return;
    }
    
    syncInProgress.store(true);
    
    // Since LSMTree doesn't have a forEach method, we need to handle this differently
    // We'll iterate through all keys in a large range
    constexpr int MIN_KEY = std::numeric_limits<int>::min();
    constexpr int MAX_KEY = std::numeric_limits<int>::max();
    
    auto entries = lsmTree.range(MIN_KEY, MAX_KEY);
    for (const auto& [key, value] : entries) {
        indexTree.insert(key, value);
    }
    
    syncInProgress.store(false);
}

void Database::syncDataStructures() {
    using namespace std::chrono_literals;
    
    while (!stopSync.load()) {
        // Sleep in shorter intervals (100ms) and check stopSync flag frequently
        // This allows faster shutdown response
        for (int i = 0; i < 50 && !stopSync.load(); i++) {
            std::this_thread::sleep_for(100ms);
        }
        
        // If stop was requested during the sleep, exit immediately
        if (stopSync.load()) {
            break;
        }
        
        // Skip if a sync is already in progress
        if (syncInProgress.load()) {
            continue;
        }
        
        // Perform the sync
        sync();
    }
    
    std::cout << "Sync thread terminated." << std::endl;
}