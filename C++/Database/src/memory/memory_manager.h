#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <iostream>
#include "memory_allocator.h"

/**
 * MemoryManager - Manages memory allocation for the database
 * 
 * This class provides an interface to the custom memory allocator
 * for efficient memory usage with reduced fragmentation.
 */
class MemoryManager {
private:
    MemoryAllocator allocator;
    
public:
    MemoryManager();
    ~MemoryManager();
    
    void* allocate(size_t size);
    void deallocate(void* ptr);
};

#endif // MEMORY_MANAGER_H