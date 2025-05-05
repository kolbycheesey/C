#ifndef BENCHMARK_FRAMEWORK_H
#define BENCHMARK_FRAMEWORK_H

#include <iostream>
#include <string>
#include <chrono>
#include <functional>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iomanip>

class BenchmarkFramework {
private:
    struct BenchmarkResult {
        std::string name;
        double elapsedMs;
        double operationsPerSecond;
        size_t iterations;
        size_t operations;
    };
    
    std::vector<BenchmarkResult> results;
    
public:
    // Run a single benchmark
    template<typename Func>
    void runBenchmark(const std::string& name, Func benchmarkFunc, size_t operations, size_t iterations = 5);
    
    // Compare multiple implementations
    template<typename... Args>
    void compareBenchmarks(const std::string& benchmarkName, size_t operations, size_t iterations, Args... benchmarkFuncs);
    
    // Print all benchmark results
    void printResults() const;
};

#include "benchmark_framework.tpp"

#endif // BENCHMARK_FRAMEWORK_H