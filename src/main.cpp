#include "kvstore/wal.hpp"
#include <iostream>

int main() {
    std::cout << "Testing WriteAheadLog binary persistence..." << std::endl;
    WriteAheadLog wal("wal_test.bin");
    wal.append_put("test_key", "test_value");
    std::cout << "WAL log entry appended successfully!" << std::endl;
    return 0;
}