#pragma once

#include "parser/ast.hxx"

#include "Type/type.hxx"
#include "symbol/symbol.hxx"
#include "symboltable/smbtable.hxx"

#include <vector>
#include <string>

class analyzer {
private:
    smbTable symbols;

    Type currentReturnType;

    bool analyzeStatement(const stmt& statement);
    Type analyzeExpression(const Expr& expression);

    bool analyzeFunction(const fnstmt& function);
    bool analyzeVariable(const varDeclStmt& variable);

    Type resolvetype(const token& type);

    void reportError(
        const std::string& message
    );

public:
    bool analyze(const program& ast);
};