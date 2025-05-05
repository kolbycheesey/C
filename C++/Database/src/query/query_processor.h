#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include <string>
#include <vector>

/**
 * QueryProcessor - Handles parsing and executing database queries
 * 
 * This class provides the interface for processing various query types
 * including SELECT, INSERT, UPDATE, and DELETE operations.
 */
class QueryProcessor {
public:
    QueryProcessor();
    ~QueryProcessor();

    // Basic query execution
    bool executeQuery(const std::string& query, std::vector<std::string>& results);
    
    // Query parsing
    bool parseQuery(const std::string& query);
    
    // Query validation
    bool validateQuery(const std::string& query);
};

#endif // QUERY_PROCESSOR_H