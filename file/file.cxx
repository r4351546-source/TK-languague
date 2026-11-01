#include "file.hxx"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::string file::readFile(const std::string& path) {
    std::ifstream input(path);

    if (!input.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::stringstream buffer;
    buffer << input.rdbuf();

    return buffer.str();
}