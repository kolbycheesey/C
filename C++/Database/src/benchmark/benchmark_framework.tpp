#ifndef BENCHMARK_FRAMEWORK_TPP
#define BENCHMARK_FRAMEWORK_TPP

#include "benchmark_framework.h"

// Run a single benchmark
template<typename Func>
void BenchmarkFramework::runBenchmark(const std::string& name, Func benchmarkFunc, size_t operations, size_t iterations) {
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
void BenchmarkFramework::compareBenchmarks(const std::string& benchmarkName, size_t operations, size_t iterations, Args... benchmarkFuncs) {
    std::cout << "===== " << benchmarkName << " Comparison =====" << std::endl;
    
    int i = 0;
    (runBenchmark(benchmarkName + " Implementation " + std::to_string(++i), 
                 benchmarkFuncs, operations, iterations), ...);
                 
    std::cout << "===== End " << benchmarkName << " Comparison =====" << std::endl << std::endl;
}

#endif // BENCHMARK_FRAMEWORK_TPP