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
        Type type,
        bool isConst = false
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
    bool isconst = false;

    unsigned long scopeDepth = 0;

    unsigned long referentScopeDepth = 0;
};
