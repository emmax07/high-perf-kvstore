#pragma once

#include "kvstore/wal.hpp"
#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>
#include <memory>

class KVEngine {
public:
    // Initializes the engine, creates the DB directory if needed, 
    // and recovers state from the WAL log file.
    explicit KVEngine(const std::string& db_dir);
    ~KVEngine() = default;

    // Core Key-Value Operations
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool remove(const std::string& key);

    // Helper utilities for inspection
    size_t size() const;
    void print_state() const;

private:
    std::string db_dir_;
    std::unordered_map<std::string, std::string> memtable_;
    
    // R/W Lock: Allows concurrent reads, but exclusive writes
    mutable std::shared_mutex rw_mutex_;
    
    std::unique_ptr<WriteAheadLog> wal_;
};