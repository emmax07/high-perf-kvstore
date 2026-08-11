#pragma once

// ============================================================================
// Cross-Platform POSIX / Windows File I/O Compatibility Layer
// ============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
    #include <fcntl.h>
    #include <sys/stat.h>

    // Map Linux POSIX function names to Windows MSVC equivalent functions
    #define open_file   ::_open
    #define close_file  ::_close
    #define read_file   ::_read
    #define write_file  ::_write
    #define fsync_file  ::_commit   // Windows equivalent of POSIX fsync()
    
    // Windows file creation flags
    #ifndef O_RDWR
        #define O_RDWR _O_RDWR
    #endif
    #ifndef O_CREAT
        #define O_CREAT _O_CREAT
    #endif
    #ifndef O_APPEND
        #define O_APPEND _O_APPEND
    #endif
    #ifndef O_RDONLY
        #define O_RDONLY _O_RDONLY
    #endif
    #ifndef O_BINARY
        #define O_BINARY _O_BINARY  // Essential on Windows to prevent auto newline translation
    #endif
#else
    // Native POSIX for Linux / macOS
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/stat.h>

    #define open_file   ::open
    #define close_file  ::close
    #define read_file   ::read
    #define write_file  ::write
    #define fsync_file  ::fsync
    #define O_BINARY    0
#endif