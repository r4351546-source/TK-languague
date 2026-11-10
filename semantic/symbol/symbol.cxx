#include "symbol.hxx"
#include <utility>

symbol::symbol(
    const std::string& name,
    symbolKind kind,
    Type type
) : name(name),
      kind(kind),
      type(type)
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