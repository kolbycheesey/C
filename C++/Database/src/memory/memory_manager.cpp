#include "memory_manager.h"

MemoryManager::MemoryManager() {
    std::cout << "Initializing custom memory manager..." << std::endl;
}

MemoryManager::~MemoryManager() {
    // Cleanup, if necessary
}

void* MemoryManager::allocate(size_t size) {
    return allocator.allocate(size);
}

void MemoryManager::deallocate(void* ptr) {
    allocator.deallocate(ptr);
}