#pragma once

enum class typekind {
    INTEGER,
    FLOAT,
    CHAR,
    BOOL,
    VOID,
    INVALID
};

struct Type {
    typekind kind;

    bool operator==(const Type& other) const {
        return kind == other.kind;
    }

    bool operator!=(const Type& other) const {
        return !(*this == other);
    }
};

namespace tktype {
    const char* typeToString(Type type);
    inline bool isCompatible(Type a, Type b) {
    return a == b;
    }
};
