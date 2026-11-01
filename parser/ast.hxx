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