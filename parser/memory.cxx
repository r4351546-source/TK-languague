#include "parser.hxx"

bool parser::isHeapAllocationDeclaration() const
{
    if (current >= tokens->size() ||
        (*tokens)[current].TokenType != tokenType::New)
    {
        return false;
    }

    unsigned long look = current + 1;

    //пропускаємо ptr, ptr, ptr...
    while (
        look < tokens->size() &&
        (*tokens)[look].TokenType == tokenType::ptr
    )
    {
        ++look;
    }

    if (look >= tokens->size())
        return false;

    //перевіряємо базовий тип
    switch ((*tokens)[look].TokenType)
    {
        case tokenType::INTEGERTYPE:
        case tokenType::FLOATTYPE:
        case tokenType::CHARTYPE:
        case tokenType::BOOLTYPE:
        case tokenType::VOIDTYPE:
            ++look;
            break;

        default:
            return false;
    }

    //якщо після типу йде ім'я  це heap декларація, а не new expression
    return look < tokens->size() &&
           (*tokens)[look].TokenType == tokenType::IDENTIFIER;
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