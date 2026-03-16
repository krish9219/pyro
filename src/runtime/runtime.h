#pragma once
#include <string>

namespace pyro {
namespace runtime {

// Pyro runtime version info
constexpr const char* VERSION = "1.0.0";
constexpr const char* AUTHOR = "Aravind Pilla";
constexpr const char* LANGUAGE = "Pyro";

// Runtime initialization
void init();
void shutdown();

// File I/O helpers
std::string read_file(const std::string& path);
void write_file(const std::string& path, const std::string& content);

} // namespace runtime
} // namespace pyro
