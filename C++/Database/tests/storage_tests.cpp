#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <cstring>
#include "../src/storage/storage_engine.h"
#include "../src/storage/mmap_manager.h"
#include "../src/utils/logger.h"

// Simple test case structure
struct TestCase {
    std::string name;
    std::function<bool()> testFunction;
};

// Test functions
bool test_storage_engine_init() {
    try {
        StorageEngine engine("test_storage");
        LOG_INFO("Storage engine initialized successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize storage engine: " + std::string(e.what()));
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
            LOG_ERROR("Failed to map data file");
            return false;
        }
        
        // Write some data to the mapped memory
        const char* testData = "Test data for memory-mapped file";
        std::memcpy(mappedMemory, testData, strlen(testData));
        LOG_DEBUG("Wrote test data to mapped memory");
        
        // Sync the data to disk
        bool syncResult = engine.syncData(filename);
        if (!syncResult) {
            LOG_ERROR("Failed to sync data to disk");
            return false;
        }
        
        LOG_INFO("Memory mapped file operations successful");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during storage test: " + std::string(e.what()));
        return false;
    }
}

bool test_mmap_direct() {
    try {
        MMapManager mmapManager;
        const std::string filename = "test_direct_map";
        const size_t fileSize = 4096; // 4KB
        
        LOG_DEBUG("Testing direct MMap operations with file: " + filename);
        
        // Map file with create flag set to true
        void* mappedMemory = mmapManager.mapFile(filename, fileSize, false, true);
        
        if (mappedMemory == nullptr) {
            LOG_ERROR("Failed to map file directly via MMapManager");
            return false;
        }
        
        // Write test data
        const char* testData = "Direct MMap test data";
        std::memcpy(mappedMemory, testData, strlen(testData));
        LOG_DEBUG("Wrote test data to direct mapped file");
        
        // Sync the data
        bool syncResult = mmapManager.syncFile(filename);
        if (!syncResult) {
            LOG_ERROR("Failed to sync memory-mapped file");
            return false;
        }
        
        // Get the mapping and verify data
        void* retrievedMapping = mmapManager.getMapping(filename);
        if (retrievedMapping == nullptr) {
            LOG_ERROR("Failed to retrieve mapping");
            return false;
        }
        
        // Clean up
        bool unmapResult = mmapManager.unmapFile(filename);
        if (!unmapResult) {
            LOG_ERROR("Failed to unmap file");
            return false;
        }
        
        LOG_INFO("Direct MMap operations successful");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during mmap test: " + std::string(e.what()));
        return false;
    }
}

// Main function - entry point for the test executable
int main() {
    // Initialize the logger with the appropriate LogLevel based on compile-time setting
    LogLevel runtimeLogLevel;
    
    #if LOG_LEVEL == LOG_LEVEL_DEBUG
        runtimeLogLevel = LogLevel::DEBUG;
    #elif LOG_LEVEL == LOG_LEVEL_INFO
        runtimeLogLevel = LogLevel::INFO;
    #elif LOG_LEVEL == LOG_LEVEL_WARNING
        runtimeLogLevel = LogLevel::WARNING;
    #elif LOG_LEVEL == LOG_LEVEL_ERROR
        runtimeLogLevel = LogLevel::ERR;
    #else
        runtimeLogLevel = LogLevel::NONE;
    #endif
    
    #ifdef LOG_TO_FILE
        #ifdef LOG_FILE_PATH
            Logger::getInstance().init(LOG_FILE_PATH, runtimeLogLevel, false);
        #else
            Logger::getInstance().init("storage_tests.log", runtimeLogLevel, false);
        #endif
    #else
        Logger::getInstance().init("", runtimeLogLevel, true);
    #endif
    
    LOG_INFO("Running storage tests...");
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
        LOG_INFO("Running test: " + test.name);
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
    LOG_INFO("Test summary: " + std::to_string(passed) + " / " + 
             std::to_string(testCases.size()) + " tests passed.");

    return (passed == testCases.size()) ? 0 : 1;
}