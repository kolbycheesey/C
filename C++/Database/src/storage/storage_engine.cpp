#include "storage_engine.h"

StorageEngine::StorageEngine(const std::string& directory) : dataDirectory(directory) {
    std::cout << "Initializing storage engine with data directory: " << directory << std::endl;
}

StorageEngine::~StorageEngine() {
    // Cleanup, if necessary
}

void* StorageEngine::mapDataFile(const std::string& name, size_t size) {
    std::string path = dataDirectory + "/" + name;
    return mmapManager.mapFile(path, size, false, true);
}

bool StorageEngine::syncData(const std::string& name) {
    std::string path = dataDirectory + "/" + name;
    return mmapManager.syncFile(path);
}