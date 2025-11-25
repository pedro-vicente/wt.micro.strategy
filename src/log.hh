#ifndef LOG_HH_
#define LOG_HH_ 1

#include <iostream>
#include <sstream>

// Simple logging macros compatible with spdlog interface
// For production, link with actual spdlog library

#define SPDLOG_TRACE(msg) do { std::cout << "[TRACE] " << msg << std::endl; } while(0)
#define SPDLOG_DEBUG(msg) do { std::cout << "[DEBUG] " << msg << std::endl; } while(0)
#define SPDLOG_INFO(msg)  do { std::cout << "[INFO] " << msg << std::endl; } while(0)
#define SPDLOG_WARN(msg)  do { std::cout << "[WARN] " << msg << std::endl; } while(0)
#define SPDLOG_ERROR(msg) do { std::cerr << "[ERROR] " << msg << std::endl; } while(0)

#endif
