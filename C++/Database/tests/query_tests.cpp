#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include "../src/query/query_processor.h"
#include "../src/utils/logger.h"

// Simple test case structure
struct TestCase {
    std::string name;
    std::function<bool()> testFunction;
};

// Test functions
bool test_query_parsing() {
    try {
        QueryProcessor processor;
        std::string queryStr = "SELECT * FROM test_table WHERE id = 1";
        
        bool result = processor.parseQuery(queryStr);
        LOG_INFO("Query parsing " + std::string(result ? "successful" : "failed"));
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during query parsing: " + std::string(e.what()));
        return false;
    }
}

bool test_query_execution() {
    try {
        QueryProcessor processor;
        std::string queryStr = "SELECT * FROM test_table";
        std::vector<std::string> results;
        
        bool result = processor.executeQuery(queryStr, results);
        LOG_INFO("Query execution " + std::string(result ? "successful" : "failed"));
        if (result) {
            LOG_DEBUG("Retrieved " + std::to_string(results.size()) + " results");
        }
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during query execution: " + std::string(e.what()));
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
            Logger::getInstance().init("query_tests.log", runtimeLogLevel, false);
        #endif
    #else
        Logger::getInstance().init("", runtimeLogLevel, true);
    #endif
    
    LOG_INFO("Running query tests...");
    std::cout << "Running query tests..." << std::endl;

    // Define test cases
    std::vector<TestCase> testCases = {
        {"Query Parsing", test_query_parsing},
        {"Query Execution", test_query_execution},
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