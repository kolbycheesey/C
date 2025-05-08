#include "memory_manager.h"
#include "../utils/logger.h"

MemoryManager::MemoryManager() {
    LOG_INFO("Initializing custom memory manager...");
}

MemoryManager::~MemoryManager() {
    LOG_DEBUG("MemoryManager destructor called");
}

void* MemoryManager::allocate(size_t size) {
    LOG_DEBUG("Allocating " + std::to_string(size) + " bytes");
    return allocator.allocate(size);
}

void MemoryManager::deallocate(void* ptr) {
    LOG_DEBUG("Deallocating memory at " + std::to_string(reinterpret_cast<uintptr_t>(ptr)));
    allocator.deallocate(ptr);
}