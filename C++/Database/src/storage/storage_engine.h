#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

#include <string>
#include <iostream>
#include "mmap_manager.h"

/**
 * StorageEngine - Manages data file storage for the database
 * 
 * Provides an interface for memory-mapped file access and durability
 * operations like flush/sync.
 */
class StorageEngine {
private:
    MMapManager mmapManager;
    std::string dataDirectory;
    
public:
    StorageEngine(const std::string& directory);
    ~StorageEngine();
    
    void* mapDataFile(const std::string& name, size_t size);
    bool syncData(const std::string& name);
};

#endif // STORAGE_ENGINE_H