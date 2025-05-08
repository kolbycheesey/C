#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include "../src/database/database.h"

// Simple test case structure
struct TestCase {
    std::string name;
    std::function<bool()> testFunction;
};

// Test functions
bool test_database_creation() {
    try {
        Database db("test_db");
        std::cout << "Created database successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create database: " << e.what() << std::endl;
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
            std::cerr << "Failed to put data into database" << std::endl;
            return false;
        }
        
        // Test get operation
        std::string retrievedValue;
        if (!db.get(key, retrievedValue)) {
            std::cerr << "Failed to get data from database" << std::endl;
            return false;
        }
        
        if (retrievedValue != value) {
            std::cerr << "Retrieved value doesn't match: expected '" 
                      << value << "', got '" << retrievedValue << "'" << std::endl;
            return false;
        }
        
        std::cout << "Put/Get operations successful" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during database operations: " << e.what() << std::endl;
        return false;
    }
}

// Main function - entry point for the test executable
int main() {
    std::cout << "Running database tests..." << std::endl;

    // Define test cases
    std::vector<TestCase> testCases = {
        {"Database Creation", test_database_creation},
        {"Database Put/Get Operations", test_database_put_get},
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