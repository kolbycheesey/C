#include "benchmark_framework.h"
#include "../utils/logger.h"
#include <sstream> // Added for std::stringstream

// Print all benchmark results
void BenchmarkFramework::printResults() const {
    std::cout << "===== Benchmark Results Summary =====" << std::endl;
    std::cout << std::left;
    std::cout << std::setw(30) << "Benchmark" << " | ";
    std::cout << std::setw(15) << "Avg Time (ms)" << " | ";
    std::cout << std::setw(15) << "Ops/sec" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (const auto& result : results) {
        std::cout << std::setw(30) << result.name << " | ";
        std::cout << std::setw(15) << result.elapsedMs << " | ";
        std::cout << std::setw(15) << result.operationsPerSecond << std::endl;
    }
    
    std::cout << "=====================================" << std::endl;

    // Also log this information at INFO level for file logging
    LOG_INFO("Benchmark Results Summary");
    for (const auto& result : results) {
        std::stringstream ss;
        ss << result.name << ": " 
           << result.elapsedMs << " ms, "
           << result.operationsPerSecond << " ops/sec";
        LOG_INFO(ss.str());
    }

} // BenchmarkFramework::printResults