#pragma once

#include "../Type/type.hxx"

#include <string>
#include <vector>
#include <memory>

enum class symbolKind {
    VARIABLE,
    FUNCTION,
    PARAMETER
};

struct symbol {
    std::string name;

    symbolKind kind;

    Type type;

    std::vector<Type> parameterTypes;

};

using smbptr = std::unique_ptr<symbol>;