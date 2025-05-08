# Purpose

The purpose of this project is to attempt to make a Database that is all about data loss prevention

# Next Steps

Add Write-Ahead Logging

For durability of in-memory data against crashes
Implement Bloom Filters

To quickly determine if a key might exist in an SSTable
Create a Database Manager

Integrate our LSM-Tree and B-Tree components (Make LSM write only and B-Tree read only)
Add a query interface for users
Add Transaction Support

Implement MVCC (Multi-Version Concurrency Control) for isolation

# Build Instructions
## From base project directory
### Build base program
mkdir build (if it doesn't exist)
cd build
cmake ..
cmake --build .

### Build with Tests
cd build
cmake -DBUILD_TESTS=ON -DUSE_MONGODB=ON ..
cmake --build .

### Build all targets

### Build with logging
#### For debugging with all logs going to console
cmake -DLOG_LEVEL=DEBUG ..

#### For production with only important messages and logs to file
cmake -DLOG_LEVEL=WARNING -DLOG_TO_FILE=ON ..

# Clean Instructions
### From base project directory
cd build
cmake --build . --target purge

# Benchmarks
## Updated on all new builds
```High-Performance Database Engine Starting...
Initializing high-performance database: HighPerformanceDB
Initializing custom memory manager...
Initializing storage engine with data directory: ./data
Initializing query processor...
Running write performance benchmark (1000000 records)...
Running benchmark: B+ Tree Insertion
  - Run 1: 2512.57 ms
  - Run 2: 2503.71 ms
  - Run 3: 2515.89 ms
  - Run 4: 2549.28 ms
  - Run 5: 2517.06 ms     
  Summary:
  - Average: 2519.7 ms    
  - Median: 2515.89 ms    
  - Operations/sec: 396873

Running benchmark: LSM Tree Insertion
  - Run 1: 1914.68 ms
  - Run 2: 1932.93 ms
  - Run 3: 2004.93 ms
  - Run 4: 1898.54 ms
  - Run 5: 1949.7 ms
  Summary:
  - Average: 1940.16 ms
  - Median: 1932.93 ms
  - Operations/sec: 515422

Running benchmark: B+ Tree Random Lookup
  - Run 1: 118.462 ms
  - Run 2: 120.401 ms
  - Run 3: 118.399 ms
  - Run 4: 118.786 ms
  - Run 5: 118.172 ms
  Summary:
  - Average: 118.844 ms
  - Median: 118.462 ms
  - Operations/sec: 841439

Running benchmark: LSM Tree Random Lookup
  - Run 1: 195.096 ms
  - Run 2: 190.545 ms
  - Run 3: 184.639 ms
  - Run 4: 187.928 ms
  - Run 5: 186.087 ms
  Summary:
  - Average: 188.859 ms
  - Median: 187.928 ms
  - Operations/sec: 529496

Running benchmark: B+ Tree Range Query
  - Run 1: 154.412 ms
  - Run 2: 150.99 ms
  - Run 3: 150.364 ms
  - Run 4: 151.228 ms
  - Run 5: 146.84 ms
  Summary:
  - Average: 150.767 ms
  - Median: 150.99 ms
  - Operations/sec: 663.276

Running benchmark: LSM Tree Range Query
  - Run 1: 415.946 ms
  - Run 2: 348.363 ms
  - Run 3: 344.157 ms
  - Run 4: 355.159 ms
  - Run 5: 349.763 ms
  Summary:
  - Average: 362.678 ms
  - Median: 349.763 ms
  - Operations/sec: 275.727

===== Benchmark Results Summary =====
Benchmark                      | Avg Time (ms)   | Ops/sec
----------------------------------------------------------------------
B+ Tree Insertion              | 2519.7          | 396873
LSM Tree Insertion             | 1940.16         | 515422
B+ Tree Random Lookup          | 118.844         | 841439
LSM Tree Random Lookup         | 188.859         | 529496
B+ Tree Range Query            | 150.767         | 663.276
LSM Tree Range Query           | 362.678         | 275.727        
=====================================
Database engine initialized successfully.```