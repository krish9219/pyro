#include "runtime/runtime.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace pyro {
namespace runtime {

void init() {
    // Runtime initialization
}

void shutdown() {
    // Runtime cleanup
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write to file: " + path);
    }
    file << content;
}

} // namespace runtime
} // namespace pyro
