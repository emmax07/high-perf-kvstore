#include "kvstore/engine.hpp"
#include "kvstore/io_compat.hpp"
#include <iostream>
#include <direct.h>  // For _mkdir on Windows MSVC

KVEngine::KVEngine(const std::string& db_dir) 
    : db_dir_(db_dir) {
    
    // Cross-platform directory creation
    struct stat st = {0};
    if (::stat(db_dir_.c_str(), &st) == -1) {
#if defined(_WIN32) || defined(_WIN64)
        ::_mkdir(db_dir_.c_str());
#else
        ::mkdir(db_dir_.c_str(), 0755);
#endif
    }

    // Initialize WAL and automatically replay logged operations into MemTable
    std::string wal_path = db_dir_ + "/wal.bin";
    wal_ = std::make_unique<WriteAheadLog>(wal_path);
    
    std::cout << "[KVEngine] Recovering database state from WAL..." << std::endl;
    wal_->recover(memtable_);
    std::cout << "[KVEngine] Recovery complete. Loaded " << memtable_.size() << " records into MemTable." << std::endl;
}

void KVEngine::put(const std::string& key, const std::string& value) {
    // 1. Write to WAL first (Durable on disk)
    wal_->append_put(key, value);

    // 2. Acquire exclusive write lock & update MemTable (In-memory)
    std::unique_lock<std::shared_mutex> lock(rw_mutex_);
    memtable_[key] = value;
}

std::optional<std::string> KVEngine::get(const std::string& key) {
    // Shared read lock allows multiple threads to read concurrently
    std::shared_lock<std::shared_mutex> lock(rw_mutex_);

    auto it = memtable_.find(key);
    if (it != memtable_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool KVEngine::remove(const std::string& key) {
    // 1. Check existence under read lock
    {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        if (memtable_.find(key) == memtable_.end()) {
            return false;
        }
    }

    // 2. Append tombstone / delete operation to WAL
    wal_->append_delete(key);

    // 3. Exclusive write lock to erase from MemTable
    std::unique_lock<std::shared_mutex> lock(rw_mutex_);
    memtable_.erase(key);
    return true;
}

size_t KVEngine::size() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex_);
    return memtable_.size();
}

void KVEngine::print_state() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex_);
    std::cout << "--- [Current MemTable State] ---" << std::endl;
    if (memtable_.empty()) {
        std::cout << "  (empty)" << std::endl;
    } else {
        for (const auto& [k, v] : memtable_) {
            std::cout << "  " << k << " => " << v << std::endl;
        }
    }
    std::cout << "--------------------------------" << std::endl;
}