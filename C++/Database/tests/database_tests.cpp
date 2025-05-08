#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include "../src/database/database.h"
#include "../src/utils/logger.h"

// Simple test case structure
struct TestCase {
    std::string name;
    std::function<bool()> testFunction;
};

// Test functions
bool test_database_creation() {
    try {
        Database db("test_db");
        LOG_INFO("Created database successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create database: " + std::string(e.what()));
        return false;
    }
}

bool test_database_put_get() {
    try {
        Database db("test_db");
        
        // Test put operation
        int key = 42;
        std::string value = "test_value";
        if (!db.put(key, value)) {
            LOG_ERROR("Failed to put data into database");
            return false;
        }
        
        // Test get operation
        std::string retrievedValue;
        if (!db.get(key, retrievedValue)) {
            LOG_ERROR("Failed to get data from database");
            return false;
        }
        
        if (retrievedValue != value) {
            LOG_ERROR("Retrieved value doesn't match: expected '" + 
                     value + "', got '" + retrievedValue + "'");
            return false;
        }
        
        LOG_INFO("Put/Get operations successful");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during database operations: " + std::string(e.what()));
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
            Logger::getInstance().init("database_tests.log", runtimeLogLevel, false);
        #endif
    #else
        Logger::getInstance().init("", runtimeLogLevel, true);
    #endif
    
    LOG_INFO("Running database tests...");
    std::cout << "Running database tests..." << std::endl;

    // Define test cases
    std::vector<TestCase> testCases = {
        {"Database Creation", test_database_creation},
        {"Database Put/Get Operations", test_database_put_get},
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