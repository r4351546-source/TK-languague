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
    token advance();    //done
    token peek();       //done
    token previous();   //done

    bool check(tokenType type); //done
    bool match(tokenType type); //done
    token consume(tokenType type);
    bool isAtEnd();//done

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
    ExprPtr expression();   //done
    ExprPtr assignment();   //done
    ExprPtr equality();     //done
    ExprPtr comparison();   //done
    ExprPtr term();         //done
    ExprPtr factor();       //done
    ExprPtr unary();        //done
    ExprPtr call();         //done
    ExprPtr primary();      //done

public:
    parser(std::vector<token> tokens);

    program mainparse();
};