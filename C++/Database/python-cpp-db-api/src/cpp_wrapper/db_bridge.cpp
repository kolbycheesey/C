// This file contains the implementation of the database bridge that interfaces with the C++ database.
// It includes functions for connecting to the database and executing queries.

#include "db_bridge.h"
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <unordered_map>
#include <memory>

// Include the actual database headers from the main project
#include "../../../src/database/database.h"

// Global connection registry to prevent memory leaks
static std::unordered_map<std::shared_ptr<DatabaseBridge>, bool> g_connections;
static std::mutex g_connections_mutex;

DatabaseBridge::DatabaseBridge(const std::string& db_path, 
                               const std::unordered_map<std::string, std::string>& config) 
    : database_(nullptr), connected_(false) {
    try {
        // Create a real database instance using the main project's Database class
        database_ = std::make_unique<Database>(db_path);
        connected_ = true;
        std::cout << "Database bridge connected to: " << db_path << std::endl;
    } catch (const std::exception& e) {
        last_error_ = std::string("Connection error: ") + e.what();
        connected_ = false;
    }
}

DatabaseBridge::~DatabaseBridge() {
    // Clean up resources
    database_.reset();
}

bool DatabaseBridge::put(int key, const std::string& value) {
    if (!connected_ || !database_) {
        last_error_ = "Not connected to database";
        return false;
    }
    
    try {
        return database_->put(key, value);
    } catch (const std::exception& e) {
        last_error_ = std::string("Error in put operation: ") + e.what();
        return false;
    }
}

std::optional<std::string> DatabaseBridge::get(int key) const {
    if (!connected_ || !database_) {
        last_error_ = "Not connected to database";
        return std::nullopt;
    }
    
    try {
        std::string value;
        if (database_->get(key, value)) {
            return value;
        } else {
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        last_error_ = std::string("Error in get operation: ") + e.what();
        return std::nullopt;
    }
}

bool DatabaseBridge::remove(int key) {
    if (!connected_ || !database_) {
        last_error_ = "Not connected to database";
        return false;
    }
    
    try {
        return database_->remove(key);
    } catch (const std::exception& e) {
        last_error_ = std::string("Error in remove operation: ") + e.what();
        return false;
    }
}

std::vector<std::pair<int, std::string>> DatabaseBridge::range(int start_key, int end_key) const {
    if (!connected_ || !database_) {
        last_error_ = "Not connected to database";
        return {};
    }
    
    try {
        return database_->range(start_key, end_key);
    } catch (const std::exception& e) {
        last_error_ = std::string("Error in range operation: ") + e.what();
        return {};
    }
}

bool DatabaseBridge::sync() {
    if (!connected_ || !database_) {
        last_error_ = "Not connected to database";
        return false;
    }
    
    try {
        database_->sync();
        return true;
    } catch (const std::exception& e) {
        last_error_ = std::string("Error in sync operation: ") + e.what();
        return false;
    }
}

std::unordered_map<std::string, std::string> DatabaseBridge::execute_query(
    const std::string& query, 
    const std::unordered_map<std::string, std::string>& params) {
    
    std::unordered_map<std::string, std::string> result;
    
    if (!connected_ || !database_) {
        result["error"] = "Not connected to database";
        return result;
    }
    
    try {
        // We don't have direct query support in the Database class yet
        // This is a placeholder for future query processor functionality
        result["status"] = "success";
        result["message"] = "Query interface not fully implemented yet";
        result["query"] = query;
        result["result_count"] = "0";
        
        return result;
    } catch (const std::exception& e) {
        result["error"] = std::string("Error in query execution: ") + e.what();
        return result;
    }
}

bool DatabaseBridge::is_connected() const {
    return connected_;
}

std::string DatabaseBridge::get_last_error() const {
    return last_error_;
}

// Factory function for creating connections
std::shared_ptr<DatabaseBridge> create_connection(
    const std::string& db_path,
    const std::unordered_map<std::string, std::string>& config) {
    
    auto connection = std::make_shared<DatabaseBridge>(db_path, config);
    
    // Register the connection in our global registry
    std::lock_guard<std::mutex> lock(g_connections_mutex);
    g_connections[connection] = true;
    
    return connection;
}

// Function to properly close connections
bool close_connection(std::shared_ptr<DatabaseBridge>& connection) {
    if (!connection) {
        return false;
    }
    
    // Remove from registry
    {
        std::lock_guard<std::mutex> lock(g_connections_mutex);
        g_connections.erase(connection);
    }
    
    // Reset the shared pointer to release the connection
    connection.reset();
    
    return true;
}