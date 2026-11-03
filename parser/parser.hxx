#pragma once

#include "../lexer/lexer.hxx"
#include "ast.hxx"

#include <memory>
#include <vector>

class parser {
private:
    std::vector<token> tokens;
    std::size_t current = 0;

    // Navigation
    token advance();
    token peek();
    token previous();

    bool check(tokenType type);
    bool match(tokenType type);
    token consume(tokenType type);
    bool isAtEnd();

    // Declarations
    stmtptr declaration();
    stmtptr functionDeclaration();
    stmtptr variableDeclaration();

    // Statements
    stmtptr  statement();
    stmtptr blockStatement();
    stmtptr ifStatement();
    stmtptr whileStatement();
    stmtptr forStatement();
    stmtptr returnStatement();
    stmtptr expressionStatement();

    // Expressions
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr primary();

public:
    parser(std::vector<token> tokens);

    program mainparse();
};