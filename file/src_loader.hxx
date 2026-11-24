
#pragma once


#include "../lexer/lexer.hxx"

#include <filesystem>
#include <vector>
#include <string>
#include <unordered_set>

class SourceLoader {
private:
    using Path = std::filesystem::path;
    static constexpr size_t MAX_INCLUDE_DEPTH = 256;

    void expandFile(
        const Path& path,
        std::vector<Path>& includeStack,
        std::unordered_set<Path>& importedModules,
        std::vector<token>& output
    );

    Path resolveStandardLibrary(
        const std::vector<token>& tokens,
        std::size_t& index
    );

    static std::string decodeStringLiteral(
        const std::string& literal
    );

    static Path normalizePath(const Path& path);

    static Path standardLibraryRoot();

public:
    std::vector<token> load(const Path& entryFile);
};
