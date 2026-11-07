/*
* parser/ast.hxx
*/
#pragma once
#include "../lexer/lexer.hxx"
#include <memory>

struct Expr {
    virtual ~Expr() = default;
};

using ExprPtr = std::unique_ptr<Expr>;

struct LiteralExpr : Expr {
    token value;
};

struct VariableExpr : Expr {
    token name;
};

struct BinaryExpr : Expr {
    ExprPtr left;
    token op;
    ExprPtr right;
};

struct UnaryExpr : Expr {
    token op;
    ExprPtr right;
};

struct groupingExpr : Expr {
    ExprPtr expression;
};

struct assignmentExpr : Expr {
    token name;
    ExprPtr value;
};

struct callExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
};


//instructs
struct stmt {
    virtual ~stmt() = default;
};

using stmtptr = std::unique_ptr<stmt>;

struct Blockstmt : stmt {
    std::vector<stmtptr> statements;
};

struct Ifstmt : stmt {
    ExprPtr condition;
    stmtptr thenBranch;
    stmtptr elseBranch;
};

struct varDeclStmt /*приклад: int x = 5;*/ : stmt {
    token name;
    token type;
    ExprPtr initializer;
};

struct whileStmt : stmt/*while(....) {....}*/ {
    ExprPtr condition;
    stmtptr body;
};

struct expressionstmt : stmt {
    ExprPtr expression;
};

struct forStmt : stmt/*for(....) {....}*/ {
    stmtptr initializer;
    ExprPtr condition;
    ExprPtr increment;
    stmtptr body;
};

struct returnstmt : stmt/*return ....;*/ {
    ExprPtr value;
};

struct parameters {
    token type;
    token name;
};

struct fnstmt : stmt {
    token name;
    token returnType;
    
    std::vector<parameters> parameters;
    std::vector<stmtptr> body;
};


struct program {
    std::vector<stmtptr> statements;
};