#ifndef MMAP_MANAGER_H
#define MMAP_MANAGER_H

#include <string>
#include <cstdint>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

class MMapManager {
private:
    struct FileMapping {
#ifdef _WIN32
        HANDLE fileHandle;
        HANDLE mappingHandle;
#else
        int fd;
#endif
        void* data;
        size_t size;
        bool readOnly;
    };
    
    std::unordered_map<std::string, FileMapping> mappings;

public:
    MMapManager() = default;
    ~MMapManager();

    void* mapFile(const std::string& path, size_t size, bool readOnly = false, bool create = false);
    bool unmapFile(const std::string& path);
    void* getMapping(const std::string& path);
    bool syncFile(const std::string& path);
    
    // Close all open memory mappings
    void closeAll();
};

#endif // MMAP_MANAGER_H