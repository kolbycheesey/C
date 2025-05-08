#include "logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

// Initialize static members
Logger* Logger::instance = nullptr;
std::mutex Logger::mutex;

Logger::Logger() : level(LogLevel::INFO), consoleOutput(true) {
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger& Logger::getInstance() {
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lock(mutex);
        if (instance == nullptr) {
            instance = new Logger();
        }
    }
    return *instance;
}

void Logger::init(const std::string& logFilePath, LogLevel logLevel, bool outputToConsole) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Set log level
    level = logLevel;
    
    // Set console output flag
    consoleOutput = outputToConsole;
    
    // Open log file
    if (!logFilePath.empty()) {
        logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        }
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERR, message);
}

void Logger::log(LogLevel msgLevel, const std::string& message) {
    // Check if this message should be logged based on current level
    if (static_cast<int>(msgLevel) < static_cast<int>(level)) {
        return;
    }
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream logStream;
    
    // Format: [TIME][LEVEL] Message
    #ifdef _WIN32
        // Use the thread-safe localtime_s for Windows
        struct tm timeInfo;
        localtime_s(&timeInfo, &time);
        logStream << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "]";
    #else
        // Use localtime_r for POSIX systems
        struct tm timeInfo;
        localtime_r(&time, &timeInfo);
        logStream << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "]";
    #endif
    
    // Add log level
    switch (msgLevel) {
        case LogLevel::DEBUG:
            logStream << "[DEBUG] ";
            break;
        case LogLevel::INFO:
            logStream << "[INFO ] ";
            break;
        case LogLevel::WARNING:
            logStream << "[WARN ] ";
            break;
        case LogLevel::ERR:
            logStream << "[ERROR] ";
            break;
        default:
            logStream << "[?????] ";
    }
    
    // Add the message
    logStream << message;
    
    std::string logMessage = logStream.str();
    
    // Lock to prevent concurrent writes
    std::lock_guard<std::mutex> lock(mutex);
    
    // Write to log file if it's open
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
        logFile.flush();
    }
    
    // Write to console if enabled
    if (consoleOutput) {
        // Use appropriate output stream based on log level
        if (msgLevel == LogLevel::ERR) {
            std::cerr << logMessage << std::endl;
        } else {
            std::cout << logMessage << std::endl;
        }
    }
}

void Logger::setLogLevel(LogLevel newLevel) {
    std::lock_guard<std::mutex> lock(mutex);
    level = newLevel;
}