#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>
#include <unordered_map>
#include <mutex>
#include <memory>

// Size classes for the allocator (powers of 2 for efficiency)
constexpr size_t MIN_BLOCK_SIZE = 8;    // Minimum block size (8 bytes)
constexpr size_t MAX_BLOCK_SIZE = 4096; // Maximum block size (4 KB)
constexpr size_t NUM_SIZE_CLASSES = 10; // 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096

class MemoryAllocator {
private:
    // A memory slab holds multiple fixed-size blocks
    struct Slab {
        uint8_t* memory;
        size_t blockSize;
        size_t totalBlocks;
        size_t freeBlocks;
        std::vector<uint8_t*> freeList;
        
        Slab(size_t size, size_t count);
        ~Slab();
        
        void* allocate();
        bool deallocate(void* ptr);
    };
    
    // Track slabs for each size class
    std::array<std::vector<std::unique_ptr<Slab>>, NUM_SIZE_CLASSES> sizeClassSlabs;
    std::unordered_map<void*, size_t> allocatedBlocks; // Maps ptr to size class index
    std::mutex mutex; // For thread safety
    
    // Calculate the size class for a given size
    size_t getSizeClass(size_t size);
    
    // Get the block size for a given size class
    size_t getBlockSize(size_t sizeClass);

public:
    MemoryAllocator() = default;
    
    void* allocate(size_t size);
    void deallocate(void* ptr);
};

#endif // MEMORY_ALLOCATOR_H