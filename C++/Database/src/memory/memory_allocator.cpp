#include "memory_allocator.h"
#include <algorithm>

// Slab constructor implementation
MemoryAllocator::Slab::Slab(size_t size, size_t count) : 
    blockSize(size), totalBlocks(count), freeBlocks(count) {
    // Allocate a continuous chunk of memory
    memory = new uint8_t[blockSize * totalBlocks];
    
    // Initialize free list
    freeList.reserve(totalBlocks);
    for (size_t i = 0; i < totalBlocks; ++i) {
        freeList.push_back(memory + (i * blockSize));
    }
}

// Slab destructor implementation
MemoryAllocator::Slab::~Slab() {
    delete[] memory;
}

// Slab allocation method
void* MemoryAllocator::Slab::allocate() {
    if (freeBlocks == 0) return nullptr;
    
    void* block = freeList.back();
    freeList.pop_back();
    --freeBlocks;
    return block;
}

// Slab deallocation method
bool MemoryAllocator::Slab::deallocate(void* ptr) {
    uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
    
    // Check if the pointer belongs to this slab
    if (bytePtr < memory || bytePtr >= memory + (blockSize * totalBlocks)) {
        return false;
    }
    
    // Check alignment
    if ((bytePtr - memory) % blockSize != 0) {
        return false;
    }
    
    // Add back to free list
    freeList.push_back(bytePtr);
    ++freeBlocks;
    return true;
}

// Calculate size class for a given size
size_t MemoryAllocator::getSizeClass(size_t size) {
    size_t sizeClass = 0;
    size_t blockSize = MIN_BLOCK_SIZE;
    
    while (blockSize < size && sizeClass < NUM_SIZE_CLASSES - 1) {
        blockSize *= 2;
        ++sizeClass;
    }
    
    return sizeClass;
}

// Get the block size for a given size class
size_t MemoryAllocator::getBlockSize(size_t sizeClass) {
    return MIN_BLOCK_SIZE << sizeClass;
}

// Main allocation method
void* MemoryAllocator::allocate(size_t size) {
    if (size > MAX_BLOCK_SIZE) {
        // Fall back to standard allocation for large blocks
        return ::operator new(size);
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    size_t sizeClass = getSizeClass(size);
    auto& slabs = sizeClassSlabs[sizeClass];
    
    // Try to allocate from existing slabs
    for (auto& slab : slabs) {
        void* ptr = slab->allocate();
        if (ptr) {
            allocatedBlocks[ptr] = sizeClass;
            return ptr;
        }
    }
    
    // Create a new slab if all existing slabs are full
    size_t blockSize = getBlockSize(sizeClass);
    size_t blocksPerSlab = 1024 * 1024 / blockSize; // ~1MB slab size
    auto newSlab = std::make_unique<Slab>(blockSize, blocksPerSlab);
    void* ptr = newSlab->allocate();
    allocatedBlocks[ptr] = sizeClass;
    slabs.push_back(std::move(newSlab));
    
    return ptr;
}

// Deallocation method
void MemoryAllocator::deallocate(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = allocatedBlocks.find(ptr);
    if (it == allocatedBlocks.end()) {
        // Not allocated by this allocator, assume it's from standard allocator
        ::operator delete(ptr);
        return;
    }
    
    size_t sizeClass = it->second;
    auto& slabs = sizeClassSlabs[sizeClass];
    
    // Find the slab this pointer belongs to
    for (auto& slab : slabs) {
        if (slab->deallocate(ptr)) {
            allocatedBlocks.erase(it);
            return;
        }
    }
}