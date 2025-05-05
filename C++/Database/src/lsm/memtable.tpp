#ifndef MEMTABLE_TPP
#define MEMTABLE_TPP

#include "memtable.h"
#include <algorithm>

template <typename Key, typename Value>
MemTable<Key, Value>::MemTable(size_t maxMemoryBytes, MemoryAllocator* alloc)
    : memoryUsage(0), memoryLimit(maxMemoryBytes), immutable(false), allocator(alloc) {
}

template <typename Key, typename Value>
bool MemTable<Key, Value>::put(const Key& key, const Value& value) {
    if (immutable.load()) {
        return false;  // Cannot modify an immutable memtable
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    // Calculate approximate memory usage for this entry
    // This is a simplified estimation - in real implementation we would need more precise tracking
    size_t entrySize = sizeof(key) + sizeof(value);
    
    // Check if adding this entry would exceed memory limit
    if (memoryUsage + entrySize > memoryLimit) {
        return false;
    }
    
    // Insert or update value
    auto result = data.insert_or_assign(key, value);
    
    // If it's a new insertion (not an update), increase memory usage
    if (result.second) {
        memoryUsage += entrySize;
    }
    
    return true;
}

template <typename Key, typename Value>
bool MemTable<Key, Value>::get(const Key& key, Value& value) const {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = data.find(key);
    if (it != data.end()) {
        value = it->second;
        return true;
    }
    
    return false;
}

template <typename Key, typename Value>
bool MemTable<Key, Value>::remove(const Key& key) {
    if (immutable.load()) {
        return false;  // Cannot modify an immutable memtable
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = data.find(key);
    if (it != data.end()) {
        // In a full LSM implementation we would insert a tombstone marker
        // For simplicity, we'll just remove the entry from memory
        memoryUsage -= (sizeof(key) + sizeof(it->second));
        data.erase(it);
        return true;
    }
    
    return false;
}

template <typename Key, typename Value>
void MemTable<Key, Value>::makeImmutable() {
    immutable.store(true);
}

template <typename Key, typename Value>
bool MemTable<Key, Value>::isImmutable() const {
    return immutable.load();
}

template <typename Key, typename Value>
size_t MemTable<Key, Value>::getMemoryUsage() const {
    return memoryUsage;
}

template <typename Key, typename Value>
size_t MemTable<Key, Value>::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return data.size();
}

template <typename Key, typename Value>
bool MemTable<Key, Value>::isFull() const {
    return memoryUsage >= memoryLimit;
}

template <typename Key, typename Value>
std::vector<std::pair<Key, Value>> MemTable<Key, Value>::range(
    const Key& startKey, const Key& endKey) const {
    
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<std::pair<Key, Value>> result;
    
    auto it = data.lower_bound(startKey);
    while (it != data.end() && it->first <= endKey) {
        result.emplace_back(it->first, it->second);
        ++it;
    }
    
    return result;
}

template <typename Key, typename Value>
void MemTable<Key, Value>::forEach(
    const std::function<void(const Key&, const Value&)>& func) const {
    
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& [key, value] : data) {
        func(key, value);
    }
}

#endif // MEMTABLE_TPP