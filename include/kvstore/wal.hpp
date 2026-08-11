#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdint>

enum class OpType : uint8_t {
    PUT = 1,
    DELETE = 2
};

class WriteAheadLog {
public:
    explicit WriteAheadLog(const std::string& filepath);
    ~WriteAheadLog();

    WriteAheadLog(const WriteAheadLog&) = delete;
    WriteAheadLog& operator=(const WriteAheadLog&) = delete;

    void append_put(const std::string& key, const std::string& value);
    void append_delete(const std::string& key);
    void recover(std::unordered_map<std::string, std::string>& memtable);

private:
    static void append_uint32(std::vector<uint8_t>& buf, uint32_t val);
    static uint32_t read_uint32(int fd);

    std::string filepath_;
    int fd_;
    std::mutex wal_mutex_;
};