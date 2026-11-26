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
