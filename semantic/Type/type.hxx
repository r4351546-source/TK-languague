#pragma once
#include <memory>

enum class typekind {
    INTEGER,
    FLOAT,
    CHAR,
    BOOL,
    VOID,
    INVALID
};

struct Type {
    typekind kind = typekind::INVALID;

    bool isPtr = false;
    bool isRef = false;

    std::shared_ptr<Type> pointeeType;


    bool operator==(const Type& other) const {

        if (
            kind != other.kind ||
            isPtr != other.isPtr ||
            isRef != other.isRef
        ) {
            return false;
        }

        if (
            static_cast<bool>(pointeeType) !=
            static_cast<bool>(other.pointeeType)
        ) {
            return false;
        }

        if (!pointeeType) {
            return true;
        }

        return *pointeeType == *other.pointeeType;
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
