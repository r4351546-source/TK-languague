
#include "src_loader.hxx"
#include "file.hxx"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

std::filesystem::path SourceLoader::normalizePath(
    const Path& path
)
{
    return std::filesystem::weakly_canonical(
        std::filesystem::absolute(path)
    );
}



std::string SourceLoader::decodeStringLiteral(
    const std::string& literal
)
{
    if (
        literal.size() < 2 ||
        literal.front() != '"' ||
        literal.back() != '"'
    )
    {
        throw std::runtime_error(
            "Invalid include string literal"
        );
    }

    std::string result;

    for (std::size_t i = 1; i < literal.size() - 1; ++i)
    {
        char c = literal[i];

        if (c == '\\' && i + 1 < literal.size() - 1)
        {
            char next = literal[i + 1];

            if (next == '"' || next == '\\')
            {
                result += next;
                ++i;
                continue;
            }
        }

        result += c;
    }

    if (result.empty())
    {
        throw std::runtime_error(
            "Include path cannot be empty"
        );
    }

    return result;
}


// Platform-specific standard library root.
SourceLoader::Path SourceLoader::standardLibraryRoot()
{
#ifdef _WIN32

    return Path("C:/TKlang/tklib/binaries");

#else

    const char* home = std::getenv("HOME");

    if (!home || *home == '\0')
    {
        throw std::runtime_error(
            "Cannot resolve HOME environment variable"
        );
    }

    return Path(home)
        / ".TKlang"
        / "tklib"
        / "binaries";

#endif
}


// Resolve tklib::ios into binaries/ios.tk.
SourceLoader::Path SourceLoader::resolveStandardLibrary(
    const std::vector<token>& tokens,
    std::size_t& index
)
{
    std::vector<std::string> components;

    if (
        index >= tokens.size() ||
        tokens[index].TokenType != tokenType::IDENTIFIER
    )
    {
        throw std::runtime_error(
            "Expected library namespace after use"
        );
    }

    components.push_back(tokens[index++].lexeme);

    while (
        index < tokens.size() &&
        tokens[index].TokenType == tokenType::scope
    )
    {
        ++index;

        if (
            index >= tokens.size() ||
            tokens[index].TokenType != tokenType::IDENTIFIER
        )
        {
            throw std::runtime_error(
                "Expected library name after ::"
            );
        }

        components.push_back(tokens[index++].lexeme);
    }

    if (
        components.size() != 2 ||
        components[0] != "tklib"
    )
    {
        throw std::runtime_error(
            "Invalid standard library path"
        );
    }

    Path result = standardLibraryRoot();

    result /= components[1] + ".tk";

    return result;
}

// Recursively expand source files.
void SourceLoader::expandFile(
    const Path& path,
    std::vector<Path>& includeStack,
    std::unordered_set<Path>& importedModules,
    std::vector<token>& output
)
{
    if (includeStack.size() >= MAX_INCLUDE_DEPTH)
    {
        throw std::runtime_error(
            "Maximum include depth exceeded: " +
            path.string()
        );
    }

    Path absolutePath = normalizePath(path);

    // Detect circular dependencies.
    if (
        std::find(
            includeStack.begin(),
            includeStack.end(),
            absolutePath
        ) != includeStack.end()
    )
    {
        throw std::runtime_error(
            "Circular dependency detected: " +
            absolutePath.string()
        );
    }

    includeStack.push_back(absolutePath);

    try
    {
        file reader;

        std::string source = reader.readFile(
            absolutePath.string()
        );

        lexer tokenizer;

        std::vector<token> tokens =
            tokenizer.mainlexer(source);

        std::size_t i = 0;

        while (
            i < tokens.size() &&
            tokens[i].TokenType != tokenType::eof
        )
        {
            /*
             * INC
             *
             * inc "file.tk";
             */

            if (tokens[i].TokenType == tokenType::INC)
            {
                ++i;

                if (
                    i >= tokens.size() ||
                    tokens[i].TokenType != tokenType::STRING_LITERAL
                )
                {
                    throw std::runtime_error(
                        "Expected string literal after inc"
                    );
                }

                Path relativePath(
                    decodeStringLiteral(tokens[i].lexeme)
                );

                ++i;

                if (
                    i >= tokens.size() ||
                    tokens[i].TokenType != tokenType::semicolon
                )
                {
                    throw std::runtime_error(
                        "Expected ; after inc"
                    );
                }

                ++i;

                Path target = relativePath.is_absolute()
                    ? relativePath
                    : absolutePath.parent_path() / relativePath;

                expandFile(
                    target,
                    includeStack,
                    importedModules,
                    output
                );

                continue;
            }

            /*
             * USE
             *
             * use tklib::ios;
             */

            if (tokens[i].TokenType == tokenType::USE)
            {
                ++i;

                Path target = resolveStandardLibrary(
                    tokens,
                    i
                );

                if (
                    i >= tokens.size() ||
                    tokens[i].TokenType != tokenType::semicolon
                )
                {
                    throw std::runtime_error(
                        "Expected ; after use"
                    );
                }

                ++i;

                Path normalizedTarget =
                    normalizePath(target);

                //already successfully imported?
                if (
                    importedModules.find(normalizedTarget) !=
                    importedModules.end()
                )
                {
                    continue;
                }

                // Expand first. Register only after success.
                expandFile(
                    normalizedTarget,
                    includeStack,
                    importedModules,
                    output
                );

                importedModules.insert(normalizedTarget);

                continue;
            }

            
            output.push_back(tokens[i]);

            ++i;
        }
    }
    catch (...)
    {
        includeStack.pop_back();

        throw;
    }

    includeStack.pop_back();
}


std::vector<token> SourceLoader::load(
    const Path& entryFile
)
{
    std::vector<token> output;

    std::vector<Path> includeStack;

    std::unordered_set<Path> importedModules;

    expandFile(
        entryFile,
        includeStack,
        importedModules,
        output
    );

    output.push_back({
        tokenType::eof,
        ""
    });

    return output;
}