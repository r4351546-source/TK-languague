#include "symbol.hxx"
#include <utility>

symbol::symbol(
    const std::string& name,
    symbolKind kind,
    Type type,
    bool isConst
) : name(name),
      kind(kind),
      type(type),
      isconst(isConst)
{}

symbol::symbol(
    const std::string& name,
    Type returnType,
    std::vector<Type> parameterTypes
) : name(name),
      kind(symbolKind::FUNCTION),
      type(returnType),
      parameterTypes(std::move(parameterTypes))
{}