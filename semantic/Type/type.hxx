#pragma once

#include <memory>
#include <string>

enum class typekind {
    INTEGER,
    FLOAT,
    CHAR,
    BOOL,
    VOID,
    STRING,
    NULLPTR,
    INVALID
};

struct Type {
    typekind kind = typekind::INVALID;

    bool isPtr = false;
    bool isRef = false;

    std::shared_ptr<Type> pointeeType;

    static Type pointerTo(const Type& pointee) {
        Type result;

        result.kind = pointee.kind;
        result.isPtr = true;
        result.pointeeType =
            std::make_shared<Type>(pointee);

        return result;
    }

    static Type referenceTo(const Type& referred) {
        Type result;

        result.kind = referred.kind;
        result.isRef = true;
        result.pointeeType =
            std::make_shared<Type>(referred);

        return result;
    }
    bool isReference() const {
        return isRef && pointeeType != nullptr;
    }

    bool isValidReference() const {
    if (!isRef || !pointeeType)
        return false;

    Type referred = pointeeType->valueType();

    return referred.kind != typekind::VOID &&
           referred.kind != typekind::INVALID &&
           !referred.isRef;
    }

    Type valueType() const {
        if (isRef && pointeeType) {
            return pointeeType->valueType();
        }

        return *this;
    }

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

    std::string typeToString(const Type& type);

    bool isCompatible(
        const Type& expected,
        const Type& actual
    );
}
