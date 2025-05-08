#include <iostream>
#include <string>
#include <chrono>
#include "../src/benchmark/benchmark_framework.h"
#include "../src/database/database.h"
#include "../src/utils/logger.h"

// Include MongoDB headers conditionally
#ifdef USE_MONGODB
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#endif

// Main benchmark function that was previously inside Database class
void runDatabaseBenchmark() {
    Database db("HighPerformanceDB");
    BenchmarkFramework benchmarker;
    const int RECORD_COUNT = 1000000;
    
    std::cout << "Running hybrid storage benchmark (" << RECORD_COUNT << " records)..." << std::endl;
    
    // Hybrid write benchmark (LSM Tree only)
    benchmarker.runBenchmark("Hybrid Write (LSM Tree)", [&db, RECORD_COUNT]() {
        for (int i = 0; i < RECORD_COUNT; i++) {
            db.put(i, "value-" + std::to_string(i));
        }
    }, RECORD_COUNT);
    
    // Sync data from LSM Tree to B+Tree
    std::cout << "Syncing data from LSM Tree to B+Tree..." << std::endl;
    db.sync();
    
    // Hybrid read benchmark (B+Tree with LSM fallback)
    benchmarker.runBenchmark("Hybrid Read", [&db, RECORD_COUNT]() {
        // Random access pattern
        for (int i = 0; i < 100000; i++) {
            int key = rand() % RECORD_COUNT;
            std::string value;
            db.get(key, value);
        }
    }, 100000);
    
    // Hybrid range query benchmark
    benchmarker.runBenchmark("Hybrid Range Query", [&db]() {
        // Perform 100 range queries
        for (int i = 0; i < 100; i++) {
            int start = i * 1000;
            int end = start + 999;
            auto results = db.range(start, end);
        }
    }, 100);
    
    // Print all benchmark results
    benchmarker.printResults();
    std::cout << "Benchmark function completed, returning to main..." << std::endl;
}

#ifdef USE_MONGODB
// MongoDB benchmarks that mirror the custom database benchmarks
void runMongoDBBenchmark() {
    BenchmarkFramework benchmarker;
    const int RECORD_COUNT = 1000000;
    
    std::cout << "Running MongoDB benchmark (" << RECORD_COUNT << " records)..." << std::endl;
    
    // Initialize MongoDB
    mongocxx::instance instance{}; // Required for driver initialization
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    
    auto db = client["benchmark_db"];
    auto collection = db["benchmark_collection"];
    
    // Clean up any existing data
    collection.drop();
    
    // MongoDB write benchmark
    benchmarker.runBenchmark("MongoDB Write", [&collection, RECORD_COUNT]() {
        for (int i = 0; i < RECORD_COUNT; i++) {
            using bsoncxx::builder::stream::document;
            auto doc = document{} << "_id" << i << "value" << ("value-" + std::to_string(i)) << bsoncxx::builder::stream::finalize;
            collection.insert_one(doc.view());
        }
    }, RECORD_COUNT);
    
    // MongoDB read benchmark (random access pattern)
    benchmarker.runBenchmark("MongoDB Read", [&collection, RECORD_COUNT]() {
        for (int i = 0; i < 100000; i++) {
            int key = rand() % RECORD_COUNT;
            using bsoncxx::builder::stream::document;
            auto doc = document{} << "_id" << key << bsoncxx::builder::stream::finalize;
            auto result = collection.find_one(doc.view());
        }
    }, 100000);
    
    // MongoDB range query benchmark
    benchmarker.runBenchmark("MongoDB Range Query", [&collection]() {
        for (int i = 0; i < 100; i++) {
            int start = i * 1000;
            int end = start + 999;
            
            using bsoncxx::builder::stream::document;
            auto filter = document{} << "_id" << open_document 
                                     << "$gte" << start 
                                     << "$lte" << end 
                                     << close_document 
                                     << bsoncxx::builder::stream::finalize;
            
            auto cursor = collection.find(filter.view());
            std::vector<bsoncxx::document::value> results;
            for (auto&& doc : cursor) {
                results.push_back(bsoncxx::document::value(doc));
            }
        }
    }, 100);
    
    // Print all benchmark results
    benchmarker.printResults();
    std::cout << "MongoDB benchmark completed, returning to main..." << std::endl;
}
#endif

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
            Logger::getInstance().init("benchmark.log", runtimeLogLevel, false);
        #endif
    #else
        Logger::getInstance().init("", runtimeLogLevel, true);
    #endif
    
    LOG_INFO("Starting database benchmark tests...");
    
    std::cout << "Running database benchmarks..." << std::endl;
    
    // Run custom database benchmarks
    runDatabaseBenchmark();
    std::cout << "Custom database benchmarks completed." << std::endl;
    
#ifdef USE_MONGODB
    // Run MongoDB benchmarks if enabled
    std::cout << "\nRunning MongoDB benchmarks..." << std::endl;
    try {
        runMongoDBBenchmark();
    } catch (const std::exception& e) {
        LOG_ERROR("MongoDB benchmark failed: " + std::string(e.what()));
        std::cerr << "MongoDB benchmark failed: " << e.what() << std::endl;
    }
#endif
    
    LOG_INFO("All benchmarks completed successfully!");
    std::cout << "All benchmarks completed." << std::endl;
    std::cout << "High-Performance Database Engine Stopped." << std::endl;
    return 0;
}