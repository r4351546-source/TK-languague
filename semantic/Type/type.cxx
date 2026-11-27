#include "type.hxx"

std::string tktype::typeToString(const Type& type) {

    if (type.isPtr) {
        if (!type.pointeeType)
            return "invalid pointer";

        return "ptr " + typeToString(*type.pointeeType);
    }

    if (type.isRef) {
        if (!type.pointeeType)
            return "invalid reference";

        return typeToString(*type.pointeeType) + "&";
    }

    switch (type.kind) {

        case typekind::STRING:
            return "string";

        case typekind::INTEGER:
            return "int";

        case typekind::FLOAT:
            return "float";

        case typekind::CHAR:
            return "char";

        case typekind::BOOL:
            return "bool";

        case typekind::VOID:
            return "void";

        case typekind::NULLPTR:
            return "nullptr";

        default:
            return "invalid";
    }
}

bool tktype::isCompatible(const Type& expected,
        const Type& actual) {
            if (
        actual.kind == typekind::NULLPTR &&
        !actual.isPtr &&
        !actual.isRef
        ) {
        return expected.isPtr &&
               !expected.isRef &&
               expected.pointeeType;
        }

    if (
        expected.kind == typekind::NULLPTR &&
        !expected.isPtr &&
        !expected.isRef
    ) {
        return actual.isPtr &&
               !actual.isRef &&
               actual.pointeeType;
    }

    //  не можна змішувати звичайні типи
    //  посилання і вказівники
    if (expected.isPtr != actual.isPtr)
        return false;

    if (expected.isRef != actual.isRef)
        return false;

    //перевірка структури вказівника
    if (expected.isPtr) {
        if (
            !expected.pointeeType ||
            !actual.pointeeType
        ) {
            return false;
        }

        return isCompatible(
            *expected.pointeeType,
            *actual.pointeeType
        );
    }

    // Перевірка структури посилання.
    if (expected.isRef) {
        if (
            !expected.pointeeType ||
            !actual.pointeeType
        ) {
            return false;
        }

        return isCompatible(
            *expected.pointeeType,
            *actual.pointeeType
        );
    }

    return expected.kind == actual.kind;
        }