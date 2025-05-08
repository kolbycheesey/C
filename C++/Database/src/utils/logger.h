#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <iostream>

// Define integer constants for log levels that can be used in preprocessor
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_NONE 4

// Default log level if not specified
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

// Log levels enum (used at runtime)
enum class LogLevel {
    DEBUG = LOG_LEVEL_DEBUG,
    INFO = LOG_LEVEL_INFO,
    WARNING = LOG_LEVEL_WARNING,
    ERR = LOG_LEVEL_ERROR,
    NONE = LOG_LEVEL_NONE,  // Use to disable all logging
};

class Logger {
private:
    static Logger* instance;
    static std::mutex mutex;
    
    std::ofstream logFile;
    LogLevel level;
    bool consoleOutput;
    
    Logger();  // Private constructor for singleton
    
public:
    ~Logger();
    
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Get singleton instance
    static Logger& getInstance();
    
    // Initialize logger with configuration
    void init(const std::string& logFilePath, LogLevel logLevel = LogLevel::INFO, bool outputToConsole = true);
    
    // Log methods for different levels
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    // Generic log method
    void log(LogLevel msgLevel, const std::string& message);
    
    // Set current log level at runtime
    void setLogLevel(LogLevel newLevel);
};

// Convenience macros for logging
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(message) Logger::getInstance().debug(message)
#else
#define LOG_DEBUG(message) do {} while(0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(message) Logger::getInstance().info(message)
#else
#define LOG_INFO(message) do {} while(0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(message) Logger::getInstance().warning(message)
#else
#define LOG_WARNING(message) do {} while(0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(message) Logger::getInstance().error(message)
#else
#define LOG_ERROR(message) do {} while(0)
#endif

#endif // LOGGER_H