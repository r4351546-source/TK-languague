#include "parser.hxx"
#include <stdexcept>

stmtptr parser::heapAllocationDeclaration()
{
    typeinfo allocatedType = parseType();

    if (allocatedType.isref)
    {
        throw std::runtime_error(
            "Cannot allocate a reference"
        );
    }

    if (
        allocatedType.baseType.TokenType == tokenType::VOIDTYPE &&
        allocatedType.pointerDepth == 0
    )
    {
        throw std::runtime_error(
            "Cannot allocate void"
        );
    }

    token name = consume(tokenType::IDENTIFIER);

    consume(tokenType::ASSIGN);

    ExprPtr initializer = expression();

    consume(tokenType::semicolon);

    // Heap allocation завжди повертає pointer.
    ++allocatedType.pointerDepth;

    auto declaration = std::make_unique<varDeclStmt>();

    declaration->type = allocatedType;
    declaration->name = name;
    declaration->initializer = std::move(initializer);

    declaration->isHeapAllocated = true;

    return declaration;
}

stmtptr parser::heapAllocationDeclaration()
{
    typeinfo allocatedType = parseType();

    token name = consume(tokenType::IDENTIFIER);

    consume(tokenType::ASSIGN);

    ExprPtr initializer = expression();

    consume(tokenType::semicolon);

    auto declaration =
        std::make_unique<varDeclStmt>();

    declaration->type = allocatedType;
    declaration->name = name;
    declaration->initializer = std::move(initializer);

    declaration->isHeapAllocated = true;

    return declaration;
}



bool parser::isPostfixDereference() const
{
    if (current >= tokens->size())
        return false;

    if ((*tokens)[current].TokenType != tokenType::MULTIPLY)
        return false;

    if (current + 1 >= tokens->size())
        return false;

    const tokenType next =
        (*tokens)[current + 1].TokenType;

    switch (next)
    {
        case tokenType::semicolon:
        case tokenType::rightparen:
        case tokenType::rightbrace:
        case tokenType::comma:
        case tokenType::eof:

        //oператори після завершеного розіменування
        case tokenType::ASSIGN:
        case tokenType::PLUSASSIGN:
        case tokenType::MINUSASSIGN:
        case tokenType::MULTIPLYASSIGN:
        case tokenType::DIVIDEASSIGN:
        case tokenType::PLUS:
        case tokenType::MINUS:
        case tokenType::DIVIDE:
        case tokenType::EQUAL:
        case tokenType::NOTEQUAL:
        case tokenType::LESS:
        case tokenType::GREATER:
        case tokenType::LESSEQUAL:
        case tokenType::GREATEREQUAL:
        case tokenType::AND:
        case tokenType::OR:
            return true;

        default:
            return false;
    }
}