#include "mmap_manager.h"
#include "../utils/logger.h"
#include <system_error>

MMapManager::~MMapManager() {
    // Unmap all files on destruction
    for (auto& [path, mapping] : mappings) {
        unmapFile(path);
    }
}

void* MMapManager::mapFile(const std::string& path, size_t size, bool readOnly, bool create) {
#ifdef _WIN32
    // Windows implementation
    DWORD access = readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
    DWORD shareMode = FILE_SHARE_READ;
    DWORD creationDisposition = create ? OPEN_ALWAYS : OPEN_EXISTING;
    DWORD flags = FILE_ATTRIBUTE_NORMAL;

    HANDLE fileHandle = CreateFileA(
        path.c_str(),
        access,
        shareMode,
        NULL,
        creationDisposition,
        flags,
        NULL
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to open file: " + path + " - Error code: " + std::to_string(GetLastError()));
        return nullptr;
    }

    // Set file size if creating
    if (create) {
        LARGE_INTEGER fileSize;
        fileSize.QuadPart = size;
        
        if (!SetFilePointerEx(fileHandle, fileSize, NULL, FILE_BEGIN) ||
            !SetEndOfFile(fileHandle)) {
            LOG_ERROR("Failed to resize file: " + path + " - Error code: " + std::to_string(GetLastError()));
            CloseHandle(fileHandle);
            return nullptr;
        }
    }

    // Create file mapping
    DWORD protectFlag = readOnly ? PAGE_READONLY : PAGE_READWRITE;
    HANDLE mappingHandle = CreateFileMappingA(
        fileHandle,
        NULL,
        protectFlag,
        0,
        0,
        NULL
    );

    if (mappingHandle == NULL) {
        LOG_ERROR("Failed to create file mapping: " + path + " - Error code: " + std::to_string(GetLastError()));
        CloseHandle(fileHandle);
        return nullptr;
    }

    // Map view of file
    DWORD mapAccess = readOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;
    void* data = MapViewOfFile(
        mappingHandle,
        mapAccess,
        0,
        0,
        0  // Map entire file
    );

    if (data == NULL) {
        LOG_ERROR("Failed to map view of file: " + path + " - Error code: " + std::to_string(GetLastError()));
        CloseHandle(mappingHandle);
        CloseHandle(fileHandle);
        return nullptr;
    }

    // Store mapping info
    mappings[path] = {fileHandle, mappingHandle, data, size, readOnly};
    LOG_DEBUG("Successfully mapped file: " + path + " (" + std::to_string(size) + " bytes, " + (readOnly ? "read-only" : "read-write") + ")");

#else
    // POSIX implementation
    int flags = readOnly ? O_RDONLY : O_RDWR;
    if (create) {
        flags |= O_CREAT;
    }
    
    int fd = open(path.c_str(), flags, 0644);
    if (fd == -1) {
        LOG_ERROR("Failed to open file: " + path + " - " + std::system_category().message(errno));
        return nullptr;
    }
    
    // Ensure the file is the right size
    if (create) {
        if (ftruncate(fd, size) == -1) {
            LOG_ERROR("Failed to resize file: " + path + " - " + std::system_category().message(errno));
            close(fd);
            return nullptr;
        }
    }
    
    // Map the file into memory
    int prot = readOnly ? PROT_READ : (PROT_READ | PROT_WRITE);
    void* data = mmap(NULL, size, prot, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        LOG_ERROR("Failed to memory map file: " + path + " - " + std::system_category().message(errno));
        close(fd);
        return nullptr;
    }
    
    // Store mapping info
    mappings[path] = {fd, data, size, readOnly};
    LOG_DEBUG("Successfully mapped file: " + path + " (" + std::to_string(size) + " bytes, " + (readOnly ? "read-only" : "read-write") + ")");
    
    // Advise the kernel that we'll access this memory sequentially
    madvise(data, size, MADV_SEQUENTIAL);
#endif

    return data;
}

bool MMapManager::unmapFile(const std::string& path) {
    auto it = mappings.find(path);
    if (it == mappings.end()) {
        LOG_WARNING("Attempted to unmap non-existent file: " + path);
        return false;
    }
    
    auto& mapping = it->second;

#ifdef _WIN32
    // Windows implementation
    if (!UnmapViewOfFile(mapping.data)) {
        LOG_ERROR("Failed to unmap file: " + path + " - Error code: " + std::to_string(GetLastError()));
        return false;
    }
    
    CloseHandle(mapping.mappingHandle);
    CloseHandle(mapping.fileHandle);
#else
    // POSIX implementation
    if (munmap(mapping.data, mapping.size) != 0) {
        LOG_ERROR("Failed to unmap file: " + path + " - " + std::system_category().message(errno));
        return false;
    }
    
    close(mapping.fd);
#endif
    
    mappings.erase(it);
    LOG_DEBUG("Successfully unmapped file: " + path);
    return true;
}

void* MMapManager::getMapping(const std::string& path) {
    auto it = mappings.find(path);
    if (it == mappings.end()) {
        LOG_DEBUG("Attempted to get mapping for non-existent file: " + path);
        return nullptr;
    }
    return it->second.data;
}

bool MMapManager::syncFile(const std::string& path) {
    auto it = mappings.find(path);
    if (it == mappings.end() || it->second.readOnly) {
        LOG_WARNING("Cannot sync file (not mapped or read-only): " + path);
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation
    if (!FlushViewOfFile(it->second.data, it->second.size) || 
        !FlushFileBuffers(it->second.fileHandle)) {
        LOG_ERROR("Failed to sync file: " + path + " - Error code: " + std::to_string(GetLastError()));
        return false;
    }
#else
    // POSIX implementation
    if (msync(it->second.data, it->second.size, MS_SYNC) != 0) {
        LOG_ERROR("Failed to sync file: " + path + " - " + std::system_category().message(errno));
        return false;
    }
#endif
    
    LOG_DEBUG("Successfully synced file: " + path);
    return true;
}

void MMapManager::closeAll() {
    LOG_INFO("MMapManager: Closing all memory mappings (" + std::to_string(mappings.size()) + " files)...");
    
    // Make a copy of the keys to avoid iterator invalidation during unmapFile calls
    std::vector<std::string> paths;
    paths.reserve(mappings.size());
    
    for (const auto& [path, _] : mappings) {
        paths.push_back(path);
    }
    
    // Unmap all files
    for (const auto& path : paths) {
        LOG_DEBUG("Unmapping file: " + path);
        unmapFile(path);
    }
    
    // Clear any remaining mappings (should be empty already)
    mappings.clear();
    
    LOG_INFO("All memory mappings closed.");
}