# High-Performance C++ Key-Value Store (`high-perf-kvstore`)

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

A lightweight, thread-safe, persistent Key-Value storage engine built in modern C++17. Designed with a **Write-Ahead Logging (WAL)** architecture and an in-memory **MemTable** guarded by reader-writer locks, this engine provides fast in-memory operations paired with crash-resilient disk durability.

---

## Architecture Overview

The database uses a hybrid in-memory + write-ahead log architecture designed for high throughput and durability:

              +-----------------------------------+
              |             Client                |
              +-----------------------------------+
                                |
                   [ put() / get() / remove() ]
                                |
                                v
                 +-----------------------------+
                 |          KVEngine           |
                 +-----------------------------+
                    /                       \
                   /                         \

1. Write Log / \ 2. Update Memory
   v v
   +--------------------+ +---------------------+
   | WriteAheadLog | | MemTable |
   | (wal.bin) | | (std::unordered_map)|
   +--------------------+ +---------------------+
   | Disk Persistence | | Concurrent Readers |
   | Binary Append-Only | | Read/Write Mutex |
   +--------------------+ +---------------------+

### Core Components

1. **MemTable (`std::unordered_map`)**: Serves reads and writes directly in memory with average $O(1)$ complexity.
2. **Write-Ahead Log (`WriteAheadLog`)**: An append-only binary log stored on disk. Every mutation (`PUT` or `DELETE`) is flushed to disk via `fsync` before mutating the in-memory state.
3. **Crash Recovery (`recover()`)**: Upon initialization, the engine reads the binary WAL sequentially from disk to replay all logged operations and restore the exact database state into memory.
4. **Cross-Platform I/O Compatibility Layer (`io_compat.hpp`)**: A uniform abstraction wrapper bridging POSIX file I/O operations (`open`, `read`, `write`, `fsync`) and Windows MSVC equivalents (`_open`, `_read`, `_write`, `_commit`).

---

## Concurrency & Thread Safety

To maximize read throughput while guaranteeing strict thread safety:

- **Reader-Writer Locks (`std::shared_mutex`)**:
  - **Reads (`get()`)**: Acquire a `std::shared_lock`, allowing multiple reader threads to access the MemTable concurrently without blocking one another.
  - **Writes (`put()`, `remove()`)**: Acquire an exclusive `std::unique_lock`, blocking all readers and writers while modifying the internal map.
- **WAL Mutex (`std::mutex`)**: Ensures multi-threaded append operations to the write-ahead log remain atomic and free of binary interleaving or corrupt frame corruption.

---

## Binary WAL Storage Protocol

The Write-Ahead Log uses a compact binary protocol for minimum disk footprint and fast sequential recovery:

- **`PUT` Entry Format**:

  ```text
  [ 1 byte : OpType (0x01) ]
  [ 4 bytes : Big-Endian Key Length (N) ]
  [ N bytes : Raw Key String ]
  [ 4 bytes : Big-Endian Value Length (M) ]
  [ M bytes : Raw Value String ]

  ```

- **`DELETE` (Tombstone) Entry Format**:
  ```text
  [ 1 byte : OpType (0x02) ]
  [ 4 bytes : Big-Endian Key Length (N) ]
  [ N bytes : Raw Key String ]
  ```

## Build & Installation

Prerequisites:

- \*\*C++17 compliant compiler (MSVC 2019+, GCC 8+, or Clang 7+)
- \*\*CMake 3.16 or higher

## PowerShell

# 1. Clone the repository

git clone [https://github.com/your-username/high-perf-kvstore.git](https://github.com/your-username/high-perf-kvstore.git)
cd high-perf-kvstore

# 2. Generate build environment

mkdir build
cd build
cmake ..

# 3. Compile executable binaries

cmake --build .

# 4. Run the main demo application

.\Debug\kvstore_app.exe

# 5. Run unit & concurrency test suite

.\Debug\kvstore_tests.exe

## Linux / macOS

# 1. Clone the repository

git clone [https://github.com/your-username/high-perf-kvstore.git](https://github.com/your-username/high-perf-kvstore.git)
cd high-perf-kvstore

# 2. Build binaries

mkdir build && cd build
cmake ..
make -j4

# 3. Execute application & test suite

./kvstore_app
./kvstore_tests

## Testing & Verification

The suite in tests/test_kvstore.cpp covers:

- \*\* Basic CRUD Operations: Validates insertions, overwrites, reads, missing key lookups, and deletions.
- \*\* Crash Recovery Durability: Simulates artificial process crashes, instantiates a new engine instance, and verifies that state is reconstructed accurately from wal.bin.
- \*\* Multithreaded Stress Testing: Spawns multiple concurrent worker threads performing thousands of mixed read/write operations to verify lock contention safety.

## Project Structure

high-perf-kvstore/
├── CMakeLists.txt # Build instructions for CMake
├── .gitignore # Excludes binary outputs and WAL data directories
├── include/
│ └── kvstore/
│ ├── engine.hpp # Primary KVEngine public interface
│ ├── wal.hpp # WriteAheadLog binary file handler
│ └── io_compat.hpp # Cross-platform Windows/POSIX I/O macros
├── src/
│ ├── engine.cpp # Engine logic, R/W locks, recovery coordination
│ ├── wal.cpp # Binary serialization & file system persistence
│ └── main.cpp # Interactive demo executable
└── tests/
└── test_kvstore.cpp # Comprehensive test suite
