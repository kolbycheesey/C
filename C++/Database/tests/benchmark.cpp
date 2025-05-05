#ifndef BENCHMARK_FRAMEWORK_H
#define BENCHMARK_FRAMEWORK_H

#include <iostream>
#include <string>
#include <chrono>
#include <functional>
#include <vector>
#include <numeric>
#include <algorithm>

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
    void runBenchmark(const std::string& name, Func benchmarkFunc, size_t operations, size_t iterations = 5) {
        std::vector<double> durations;
        durations.reserve(iterations);
        
        std::cout << "Running benchmark: " << name << std::endl;
        
        // Warm-up run
        benchmarkFunc();
        
        // Timed runs
        for (size_t i = 0; i < iterations; ++i) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            benchmarkFunc();
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0;
            durations.push_back(duration);
            
            std::cout << "  - Run " << (i + 1) << ": " << duration << " ms" << std::endl;
        }
        
        // Calculate statistics
        double totalMs = std::accumulate(durations.begin(), durations.end(), 0.0);
        double averageMs = totalMs / iterations;
        double ops = operations * 1000.0 / averageMs;
        
        // Sort durations to find median
        std::sort(durations.begin(), durations.end());
        double medianMs = durations[iterations / 2];
        
        std::cout << "  Summary:" << std::endl;
        std::cout << "  - Average: " << averageMs << " ms" << std::endl;
        std::cout << "  - Median: " << medianMs << " ms" << std::endl;
        std::cout << "  - Operations/sec: " << ops << std::endl;
        std::cout << std::endl;
        
        // Store result
        results.push_back({name, averageMs, ops, iterations, operations});
    }
    
    // Compare multiple implementations
    template<typename... Args>
    void compareBenchmarks(const std::string& benchmarkName, size_t operations, size_t iterations, Args... benchmarkFuncs) {
        std::cout << "===== " << benchmarkName << " Comparison =====" << std::endl;
        
        int i = 0;
        (runBenchmark(benchmarkName + " Implementation " + std::to_string(++i), 
                     benchmarkFuncs, operations, iterations), ...);
                     
        std::cout << "===== End " << benchmarkName << " Comparison =====" << std::endl << std::endl;
    }
    
    // Print all benchmark results
    void printResults() const {
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
    }
};

#endif // BENCHMARK_FRAMEWORK_H