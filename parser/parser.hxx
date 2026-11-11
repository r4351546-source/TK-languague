#pragma once

#include "../lexer/lexer.hxx"
#include "ast.hxx"

#include <memory>
#include <vector>

class parser {
private:
    std::vector<token>* tokens = nullptr;
    std::size_t current = 0;

    // Navigation
    token advance();    //done
    token peek();       //done
    token previous();   //done

    bool check(tokenType type); //done
    bool match(tokenType type); //done
    token consume(tokenType type);//done
    bool isAtEnd();//done

    // Declarations
    stmtptr declaration();      //done
    stmtptr functionDeclaration();//done
    stmtptr variableDeclaration();//done

    // Statements
    stmtptr  statement();       //done
    stmtptr blockStatement();   //done
    stmtptr ifStatement();      //done
    stmtptr whileStatement();   //done
    stmtptr forStatement();     //done
    stmtptr returnStatement();  //done
    stmtptr expressionStatement();//done

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
    ExprPtr logicalAnd();   //done
    ExprPtr logicalOr();    //done

    public:
    parser(std::vector<token>* tokens);
                             
    program mainparse();
};