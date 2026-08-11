#include "kvstore/wal.hpp"
#include "kvstore/io_compat.hpp"
#include <stdexcept>
#include <vector>

WriteAheadLog::WriteAheadLog(const std::string& filepath) 
    : filepath_(filepath), fd_(-1) {
    
    fd_ = open_file(filepath_.c_str(), O_RDWR | O_CREAT | O_APPEND | O_BINARY, 0644);
    if (fd_ < 0) {
        throw std::runtime_error("File I/O Error: Failed to open WAL at " + filepath_);
    }
}

WriteAheadLog::~WriteAheadLog() {
    if (fd_ >= 0) {
        fsync_file(fd_);
        close_file(fd_);
    }
}

void WriteAheadLog::append_put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(wal_mutex_);

    uint8_t op = static_cast<uint8_t>(OpType::PUT);
    uint32_t klen = static_cast<uint32_t>(key.size());
    uint32_t vlen = static_cast<uint32_t>(value.size());

    std::vector<uint8_t> buffer;
    buffer.reserve(1 + 4 + klen + 4 + vlen);

    buffer.push_back(op);
    append_uint32(buffer, klen);
    buffer.insert(buffer.end(), key.begin(), key.end());
    append_uint32(buffer, vlen);
    buffer.insert(buffer.end(), value.begin(), value.end());

    int bytes_written = write_file(fd_, buffer.data(), static_cast<unsigned int>(buffer.size()));
    (void)bytes_written;

    fsync_file(fd_);
}

void WriteAheadLog::append_delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(wal_mutex_);

    uint8_t op = static_cast<uint8_t>(OpType::DELETE);
    uint32_t klen = static_cast<uint32_t>(key.size());

    std::vector<uint8_t> buffer;
    buffer.reserve(1 + 4 + klen);

    buffer.push_back(op);
    append_uint32(buffer, klen);
    buffer.insert(buffer.end(), key.begin(), key.end());

    int bytes_written = write_file(fd_, buffer.data(), static_cast<unsigned int>(buffer.size()));
    (void)bytes_written;

    fsync_file(fd_);
}

void WriteAheadLog::recover(std::unordered_map<std::string, std::string>& memtable) {
    int read_fd = open_file(filepath_.c_str(), O_RDONLY | O_BINARY, 0644);
    if (read_fd < 0) return;

    while (true) {
        uint8_t op_code;
        int bytes_read = read_file(read_fd, &op_code, sizeof(op_code));
        if (bytes_read <= 0) break;

        OpType op = static_cast<OpType>(op_code);

        if (op == OpType::PUT) {
            uint32_t klen = read_uint32(read_fd);
            std::string key(klen, '\0');
            read_file(read_fd, key.data(), klen);

            uint32_t vlen = read_uint32(read_fd);
            std::string value(vlen, '\0');
            read_file(read_fd, value.data(), vlen);

            memtable[key] = value;
        } 
        else if (op == OpType::DELETE) {
            uint32_t klen = read_uint32(read_fd);
            std::string key(klen, '\0');
            read_file(read_fd, key.data(), klen);

            memtable.erase(key);
        }
    }

    close_file(read_fd);
}

void WriteAheadLog::append_uint32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
    buf.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>(val & 0xFF));
}

uint32_t WriteAheadLog::read_uint32(int fd) {
    uint8_t buf[4];
    int res = read_file(fd, buf, 4);
    if (res < 4) return 0;
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8)  |
           (static_cast<uint32_t>(buf[3]));
}