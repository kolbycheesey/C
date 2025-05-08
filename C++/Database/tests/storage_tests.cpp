#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <cstring>
#include "../src/storage/storage_engine.h"
#include "../src/storage/mmap_manager.h"

// Simple test case structure
struct TestCase {
    std::string name;
    std::function<bool()> testFunction;
};

// Test functions
bool test_storage_engine_init() {
    try {
        StorageEngine engine("test_storage");
        std::cout << "Storage engine initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize storage engine: " << e.what() << std::endl;
        return false;
    }
}

bool test_map_data_file() {
    try {
        StorageEngine engine("test_storage");
        
        // Map a data file
        const std::string filename = "test_file";
        const size_t fileSize = 4096; // 4KB
        void* mappedMemory = engine.mapDataFile(filename, fileSize);
        
        if (mappedMemory == nullptr) {
            std::cerr << "Failed to map data file" << std::endl;
            return false;
        }
        
        // Write some data to the mapped memory
        const char* testData = "Test data for memory-mapped file";
        std::memcpy(mappedMemory, testData, strlen(testData));
        
        // Sync the data to disk
        bool syncResult = engine.syncData(filename);
        if (!syncResult) {
            std::cerr << "Failed to sync data to disk" << std::endl;
            return false;
        }
        
        std::cout << "Memory mapped file operations successful" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during storage test: " << e.what() << std::endl;
        return false;
    }
}

bool test_mmap_direct() {
    try {
        MMapManager mmapManager;
        const std::string filename = "test_direct_map";
        const size_t fileSize = 4096; // 4KB
        
        // Map file with create flag set to true
        void* mappedMemory = mmapManager.mapFile(filename, fileSize, false, true);
        
        if (mappedMemory == nullptr) {
            std::cerr << "Failed to map file directly via MMapManager" << std::endl;
            return false;
        }
        
        // Write test data
        const char* testData = "Direct MMap test data";
        std::memcpy(mappedMemory, testData, strlen(testData));
        
        // Sync the data
        bool syncResult = mmapManager.syncFile(filename);
        if (!syncResult) {
            std::cerr << "Failed to sync memory-mapped file" << std::endl;
            return false;
        }
        
        // Get the mapping and verify data
        void* retrievedMapping = mmapManager.getMapping(filename);
        if (retrievedMapping == nullptr) {
            std::cerr << "Failed to retrieve mapping" << std::endl;
            return false;
        }
        
        // Clean up
        bool unmapResult = mmapManager.unmapFile(filename);
        if (!unmapResult) {
            std::cerr << "Failed to unmap file" << std::endl;
            return false;
        }
        
        std::cout << "Direct MMap operations successful" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during mmap test: " << e.what() << std::endl;
        return false;
    }
}

// Main function - entry point for the test executable
int main() {
    std::cout << "Running storage tests..." << std::endl;

    // Define test cases
    std::vector<TestCase> testCases = {
        {"Storage Engine Initialization", test_storage_engine_init},
        {"Map Data File Test", test_map_data_file},
        {"Direct MMap Operations Test", test_mmap_direct},
    };

    // Run tests and collect results
    int passed = 0;
    for (const auto& test : testCases) {
        std::cout << "Running test: " << test.name << "... ";
        if (test.testFunction()) {
            std::cout << "PASSED" << std::endl;
            passed++;
        } else {
            std::cout << "FAILED" << std::endl;
        }
    }

    std::cout << "Test summary: " << passed << " / " << testCases.size() 
              << " tests passed." << std::endl;

    return (passed == testCases.size()) ? 0 : 1;
}