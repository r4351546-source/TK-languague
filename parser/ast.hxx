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
    ExprPtr target;
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
    ExprPtr target;
    token op;
    ExprPtr value;
};

struct typeinfo {
    token baseType;

    unsigned pointerDepth = 0;
    bool isref = false;
};
struct callExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
};

struct newExpr : Expr {
    typeinfo allocatedType;
    ExprPtr initializer;
};

struct delExpr : Expr {
    ExprPtr operand;
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
    typeinfo type;
    ExprPtr initializer;

    bool isconst = false;
    
    bool isHeapAllocated = false;
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
    typeinfo type;
    token name;
};

struct fnstmt : stmt {
    token name;
    typeinfo returnType;
    
    std::vector<parameters> parameters;
    std::vector<stmtptr> body;
};

struct usestmt : stmt {
    std::vector<token> path;
};

struct incstmt : stmt {
    token path;
};

struct program {
    std::vector<stmtptr> statements;
};