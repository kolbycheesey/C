#include "storage_engine.h"
#include "../utils/logger.h"

StorageEngine::StorageEngine(const std::string& directory) : dataDirectory(directory) {
    LOG_INFO("Initializing storage engine with data directory: " + directory);
}

StorageEngine::~StorageEngine() {
    LOG_DEBUG("StorageEngine destructor called");
}

void* StorageEngine::mapDataFile(const std::string& name, size_t size) {
    std::string path = dataDirectory + "/" + name;
    LOG_DEBUG("Mapping data file: " + path + " with size " + std::to_string(size) + " bytes");
    return mmapManager.mapFile(path, size, false, true);
}

bool StorageEngine::syncData(const std::string& name) {
    std::string path = dataDirectory + "/" + name;
    LOG_DEBUG("Syncing data file: " + path);
    return mmapManager.syncFile(path);
}