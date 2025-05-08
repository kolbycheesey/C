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
#include "utils/logger.h"
#include "database/database.h"

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
            Logger::getInstance().init("database.log", runtimeLogLevel, false);
        #endif
    #else
        Logger::getInstance().init("", runtimeLogLevel, true);
    #endif
    
    LOG_INFO("High-Performance Database Engine Starting...");
    
    // Create database instance
    Database db("HighPerformanceDB");
    
    // Main application code would go here instead of benchmarks
    // For example, setting up a query processing loop, REST API, etc.
    
    LOG_INFO("Database engine initialized successfully.");
    return 0;
}