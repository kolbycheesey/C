#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include "../src/query/query_processor.h"

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
        std::cout << "Query parsing " << (result ? "successful" : "failed") << std::endl;
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Exception during query parsing: " << e.what() << std::endl;
        return false;
    }
}

bool test_query_execution() {
    try {
        QueryProcessor processor;
        std::string queryStr = "SELECT * FROM test_table";
        std::vector<std::string> results;
        
        bool result = processor.executeQuery(queryStr, results);
        std::cout << "Query execution " << (result ? "successful" : "failed") << std::endl;
        if (result) {
            std::cout << "Retrieved " << results.size() << " results" << std::endl;
        }
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Exception during query execution: " << e.what() << std::endl;
        return false;
    }
}

// Main function - entry point for the test executable
int main() {
    std::cout << "Running query tests..." << std::endl;

    // Define test cases
    std::vector<TestCase> testCases = {
        {"Query Parsing", test_query_parsing},
        {"Query Execution", test_query_execution},
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