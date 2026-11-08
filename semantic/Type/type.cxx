#include "type.hxx"

const char* tktype::typeToString(Type type) {
    switch (type.kind) {
        case typekind::INTEGER: return "int";
        case typekind::FLOAT: return "float";
        case typekind::CHAR: return "char";
        case typekind::BOOL: return "bool";
        case typekind::VOID: return "void";
        default: return "invalid";
    }
}