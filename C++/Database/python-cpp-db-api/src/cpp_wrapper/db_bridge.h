#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <optional>
#include <unordered_map>

// Forward declaration of the Database class (in global namespace, not db::)
class Database;

class DatabaseBridge {
public:
    // Constructor and destructor
    DatabaseBridge(const std::string& db_path, const std::unordered_map<std::string, std::string>& config = {});
    ~DatabaseBridge();

    // Basic operations
    bool put(int key, const std::string& value);
    std::optional<std::string> get(int key) const;
    bool remove(int key);
    std::vector<std::pair<int, std::string>> range(int start_key, int end_key) const;
    bool sync();

    // Query execution
    std::unordered_map<std::string, std::string> execute_query(
        const std::string& query, 
        const std::unordered_map<std::string, std::string>& params = {}
    );

    // Status checks
    bool is_connected() const;
    std::string get_last_error() const;

private:
    std::unique_ptr<Database> database_;  // Updated to use global namespace
    mutable std::string last_error_; 
    bool connected_;
};

// Factory function to create database connections
std::shared_ptr<DatabaseBridge> create_connection(
    const std::string& db_path,
    const std::unordered_map<std::string, std::string>& config = {}
);

// Function to close a database connection
bool close_connection(std::shared_ptr<DatabaseBridge>& connection);