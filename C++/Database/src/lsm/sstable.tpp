#ifndef SSTABLE_TPP
#define SSTABLE_TPP

#include "sstable.h"
#include "memtable.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <ctime>

template <typename Key, typename Value>
std::unique_ptr<SSTable<Key, Value>> SSTable<Key, Value>::createFromMemTable(
    const MemTable<Key, Value>& memTable, 
    MMapManager* mmapManager,
    const std::string& directory,
    uint32_t level) {
    
    // Generate a unique filename for this SSTable
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << directory << "/sstable_L" << level << "_" << timestamp << ".db";
    std::string filePath = ss.str();
    
    // Create the file header
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create SSTable file: " + filePath);
    }
    
    // Prepare to write data
    uint64_t indexOffset = 0;
    uint32_t keyCount = 0;
    
    // Store the min and max keys for metadata
    Key minKey, maxKey;
    bool firstKey = true;
    
    // First, write the data section
    for (const auto& [key, value] : memTable) {
        if (firstKey) {
            minKey = key;
            maxKey = key;
            firstKey = false;
        } else {
            if (key < minKey) minKey = key;
            if (key > maxKey) maxKey = key;
        }
        
        // In a real implementation, we'd serialize key and value into a binary format
        // For simplicity, we'll use a basic format here
        
        // Write the key size, key, value size, and value
        uint32_t keySize = sizeof(key);
        uint32_t valueSize = sizeof(value);
        
        file.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
        file.write(reinterpret_cast<const char*>(&key), keySize);
        file.write(reinterpret_cast<const char*>(&valueSize), sizeof(valueSize));
        file.write(reinterpret_cast<const char*>(&value), valueSize);
        
        ++keyCount;
    }
    
    // Record the index offset (start of index section)
    indexOffset = file.tellp();
    
    // Now write the index entries
    // In a real implementation, we'd use a sparse index that samples keys
    // For simplicity, we'll index every key
    uint64_t dataOffset = 0;
    for (const auto& [key, value] : memTable) {
        // Size of key and value plus their size fields
        uint32_t entrySize = sizeof(key) + sizeof(value) + sizeof(uint32_t) * 2;
        
        // Write index entry (key, offset, size)
        file.write(reinterpret_cast<const char*>(&key), sizeof(key));
        file.write(reinterpret_cast<const char*>(&dataOffset), sizeof(dataOffset));
        file.write(reinterpret_cast<const char*>(&entrySize), sizeof(entrySize));
        
        dataOffset += entrySize;
    }
    
    // Finally, write the footer with metadata
    uint64_t dataSize = indexOffset;
    file.write(reinterpret_cast<const char*>(&keyCount), sizeof(keyCount));
    file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
    file.write(reinterpret_cast<const char*>(&indexOffset), sizeof(indexOffset));
    file.write(reinterpret_cast<const char*>(&level), sizeof(level));
    file.write(reinterpret_cast<const char*>(&minKey), sizeof(minKey));
    file.write(reinterpret_cast<const char*>(&maxKey), sizeof(maxKey));
    
    file.close();
    
    // Create and return an SSTable object for the newly created file
    return std::make_unique<SSTable<Key, Value>>(mmapManager, filePath);
}

template <typename Key, typename Value>
SSTable<Key, Value>::SSTable(MMapManager* mmapManager, const std::string& filePath) 
    : mmapManager(mmapManager), dataPtr(nullptr) {
    
    metadata.filePath = filePath;
    
    // Open the file with memory mapping
    // First determine the file size
    std::filesystem::path path(filePath);
    size_t fileSize = std::filesystem::file_size(path);
    
    // Memory map the whole file
    dataPtr = mmapManager->mapFile(filePath, fileSize, true); // Read-only mapping
    if (!dataPtr) {
        throw std::runtime_error("Failed to memory map SSTable file: " + filePath);
    }
    
    // Read the footer to get metadata
    char* ptr = static_cast<char*>(dataPtr);
    ptr += (fileSize - sizeof(Key) * 2 - sizeof(uint32_t) * 2 - sizeof(uint64_t) * 2);
    
    std::memcpy(&metadata.keyCount, ptr, sizeof(metadata.keyCount));
    ptr += sizeof(metadata.keyCount);
    
    std::memcpy(&metadata.dataSize, ptr, sizeof(metadata.dataSize));
    ptr += sizeof(metadata.dataSize);
    
    std::memcpy(&metadata.indexOffset, ptr, sizeof(metadata.indexOffset));
    ptr += sizeof(metadata.indexOffset);
    
    std::memcpy(&metadata.level, ptr, sizeof(metadata.level));
    ptr += sizeof(metadata.level);
    
    std::memcpy(&metadata.minKey, ptr, sizeof(metadata.minKey));
    ptr += sizeof(metadata.minKey);
    
    std::memcpy(&metadata.maxKey, ptr, sizeof(metadata.maxKey));
    
    // Load the index
    loadIndex();
}

template <typename Key, typename Value>
SSTable<Key, Value>::~SSTable() {
    // The MMapManager will handle unmapping the file
}

template <typename Key, typename Value>
void SSTable<Key, Value>::loadIndex() {
    // Read the index entries from the memory-mapped file
    char* ptr = static_cast<char*>(dataPtr);
    ptr += metadata.indexOffset;
    
    for (uint32_t i = 0; i < metadata.keyCount; ++i) {
        IndexEntry entry;
        
        std::memcpy(&entry.key, ptr, sizeof(entry.key));
        ptr += sizeof(entry.key);
        
        std::memcpy(&entry.offset, ptr, sizeof(entry.offset));
        ptr += sizeof(entry.offset);
        
        std::memcpy(&entry.size, ptr, sizeof(entry.size));
        ptr += sizeof(entry.size);
        
        index.push_back(entry);
    }
}

template <typename Key, typename Value>
int64_t SSTable<Key, Value>::findIndexEntry(const Key& key) const {
    // Binary search for the key in the index
    int64_t left = 0;
    int64_t right = static_cast<int64_t>(index.size()) - 1;
    
    while (left <= right) {
        int64_t mid = left + (right - left) / 2;
        
        if (index[mid].key == key) {
            return mid;  // Exact match
        }
        
        if (index[mid].key < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;  // Not found
}

template <typename Key, typename Value>
Value SSTable<Key, Value>::readValueAt(uint64_t offset, uint32_t size) const {
    // Navigate to the offset in the memory-mapped file
    char* ptr = static_cast<char*>(dataPtr) + offset;
    
    // Skip the key size and key
    uint32_t keySize;
    std::memcpy(&keySize, ptr, sizeof(keySize));
    ptr += sizeof(keySize) + keySize;
    
    // Read the value size
    uint32_t valueSize;
    std::memcpy(&valueSize, ptr, sizeof(valueSize));
    ptr += sizeof(valueSize);
    
    // Read the value
    Value value;
    std::memcpy(&value, ptr, sizeof(value));
    
    return value;
}

template <typename Key, typename Value>
bool SSTable<Key, Value>::mayContain(const Key& key) const {
    // In a real implementation, we'd check a Bloom filter first
    // For now, we'll just check if the key is within our range
    return key >= metadata.minKey && key <= metadata.maxKey;
}

template <typename Key, typename Value>
std::optional<Value> SSTable<Key, Value>::get(const Key& key) const {
    // Check if key might be in this table
    if (!mayContain(key)) {
        return std::nullopt;
    }
    
    // Find the key in the index
    int64_t pos = findIndexEntry(key);
    if (pos < 0) {
        return std::nullopt;
    }
    
    // Read the value from the data section
    return readValueAt(index[pos].offset, index[pos].size);
}

template <typename Key, typename Value>
std::vector<std::pair<Key, Value>> SSTable<Key, Value>::range(
    const Key& startKey, const Key& endKey) const {
    
    std::vector<std::pair<Key, Value>> result;
    
    // Check if range overlaps with this table
    if (startKey > metadata.maxKey || endKey < metadata.minKey) {
        return result;  // No overlap
    }
    
    // Find the first key >= startKey
    auto it = std::lower_bound(index.begin(), index.end(), startKey,
        [](const IndexEntry& entry, const Key& key) {
            return entry.key < key;
        });
    
    // Collect entries until we reach endKey
    while (it != index.end() && it->key <= endKey) {
        Value value = readValueAt(it->offset, it->size);
        result.emplace_back(it->key, value);
        ++it;
    }
    
    return result;
}

template <typename Key, typename Value>
const typename SSTable<Key, Value>::Metadata& 
SSTable<Key, Value>::getMetadata() const {
    return metadata;
}

template <typename Key, typename Value>
const typename SSTable<Key, Value>::IndexEntry& 
SSTable<Key, Value>::getIndexEntry(size_t pos) const {
    return index[pos];
}

template <typename Key, typename Value>
size_t SSTable<Key, Value>::getIndexSize() const {
    return index.size();
}

template <typename Key, typename Value>
const std::string& SSTable<Key, Value>::getFilePath() const {
    return metadata.filePath;
}

template <typename Key, typename Value>
void SSTable<Key, Value>::forEach(
    const std::function<void(const Key&, const Value&)>& func) const {
    
    for (const auto& entry : index) {
        Value value = readValueAt(entry.offset, entry.size);
        func(entry.key, value);
    }
}

#endif // SSTABLE_TPP