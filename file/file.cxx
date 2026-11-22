#include "file.hxx"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>


std::string file::readFile(const std::string& path) {
    // Check file extension
    std::filesystem::path filepath(path);
    std::string extension = filepath.extension().string();

    if (extension != ".tk" && extension != ".tkl") {
        throw std::runtime_error(
            "Invalid file extension: " + extension +
            ". Expected .tk or .tkl"
        );
    }

    // Open file
    std::ifstream input(path, std::ios::binary);

    if (!input.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    // Read file
    std::stringstream buffer;
    buffer << input.rdbuf();

    if (input.bad())
    {
        throw std::runtime_error(
            "Failed to read file: " + path
        );
    }

    return buffer.str();
}
