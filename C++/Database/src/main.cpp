#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "utils/error.h"
#include "database/database.h"

int main() {
    std::cout << "High-Performance Database Engine Starting..." << std::endl;
    
    // Create database instance
    Database db("HighPerformanceDB");
    
    // Run benchmarks
    db.benchmark();
    
    std::cout << "Database engine initialized successfully." << std::endl;
    return 0;
}