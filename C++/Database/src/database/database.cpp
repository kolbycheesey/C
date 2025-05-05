#include "database.h"
#include "../storage/mmap_manager.h"
#include "../storage/storage_engine.h"
#include "../query/query_processor.h"
#include <chrono>

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
    // Signal the sync thread to stop
    stopSync.store(true);
    
    // Wait for sync thread to finish
    if (syncThread.joinable()) {
        syncThread.join();
    }
}

bool Database::put(int key, const std::string& value) {
    // Write operations only go to LSM Tree for optimal write performance
    return lsmTree.put(key, value);
}

bool Database::remove(int key) {
    // For removals, we need to update both data structures to maintain consistency
    std::lock_guard<std::mutex> lock(accessMutex);
    
    // First remove from B+Tree
    indexTree.remove(key);
    
    // Then remove from LSM Tree
    return lsmTree.remove(key);
}

bool Database::get(int key, std::string& value) const {
    // Read operations primarily from B+Tree for optimal read performance
    {
        std::lock_guard<std::mutex> lock(accessMutex);
        
        // First try B+Tree
        try {
            value = indexTree.find(key);
            return true;
        } catch (...) {
            // Not found in B+Tree, try LSM Tree as fallback
        }
    }
    
    // If not in B+Tree, check LSM Tree (this might be newly written data not yet synced)
    return lsmTree.get(key, value);
}

std::vector<std::pair<int, std::string>> Database::range(int startKey, int endKey) const {
    std::vector<std::pair<int, std::string>> results;
    
    // Read operations primarily from B+Tree
    {
        std::lock_guard<std::mutex> lock(accessMutex);
        
        // Get range from B+Tree
        results = indexTree.range(startKey, endKey);
    }
    
    // Check LSM Tree for any newer values in the range
    auto lsmResults = lsmTree.range(startKey, endKey);
    
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
    
    // Iterate through all entries in LSM Tree and add to B+Tree
    lsmTree.forEach([this](const int& key, const std::string& value) {
        indexTree.insert(key, value);
    });
    
    syncInProgress.store(false);
}

void Database::syncDataStructures() {
    using namespace std::chrono_literals;
    
    while (!stopSync.load()) {
        // Sync every 5 seconds
        std::this_thread::sleep_for(5s);
        
        // Skip if a sync is already in progress
        if (syncInProgress.load()) {
            continue;
        }
        
        // Perform the sync
        sync();
    }
}

void Database::benchmark() {
    const int RECORD_COUNT = 1000000;
    
    std::cout << "Running hybrid storage benchmark (" << RECORD_COUNT << " records)..." << std::endl;
    
    // Hybrid write benchmark (LSM Tree only)
    benchmarker.runBenchmark("Hybrid Write (LSM Tree)", [this, RECORD_COUNT]() {
        for (int i = 0; i < RECORD_COUNT; i++) {
            this->put(i, "value-" + std::to_string(i));
        }
    }, RECORD_COUNT);
    
    // Sync data from LSM Tree to B+Tree
    std::cout << "Syncing data from LSM Tree to B+Tree..." << std::endl;
    sync();
    
    // Hybrid read benchmark (B+Tree with LSM fallback)
    benchmarker.runBenchmark("Hybrid Read", [this, RECORD_COUNT]() {
        // Random access pattern
        for (int i = 0; i < 100000; i++) {
            int key = rand() % RECORD_COUNT;
            std::string value;
            this->get(key, value);
        }
    }, 100000);
    
    // Hybrid range query benchmark
    benchmarker.runBenchmark("Hybrid Range Query", [this]() {
        // Perform 100 range queries
        for (int i = 0; i < 100; i++) {
            int start = i * 1000;
            int end = start + 999;
            auto results = this->range(start, end);
        }
    }, 100);
    
    // Print all benchmark results
    benchmarker.printResults();
}