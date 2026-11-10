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
    symbol(
        const std::string& name,
        symbolKind kind,
        Type type
    );

    symbol(
    const std::string& name,
    Type returnType,
    std::vector<Type> parameterTypes
    );
    std::string name;

    symbolKind kind = symbolKind::VARIABLE;

    Type type = Type{ typekind::INVALID };

    std::vector<Type> parameterTypes;

};
