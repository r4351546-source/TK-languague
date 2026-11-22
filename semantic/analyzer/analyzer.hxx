#pragma once

#include "ast.hxx"

#include "Type/type.hxx"
#include "symbol/symbol.hxx"
#include "symboltable/smbtable.hxx"

#include <vector>
#include <string>

class analyzer {
private:
    smbTable symbols;

    Type currentReturnType;
    bool insideFunction = false;
    size_t loopDepth = 0;
    size_t errorCount = 0;


    bool analyzeStatement(const stmt& statement);               //done      13th November 2026
    Type analyzeExpression(const Expr& expression);             //done      13th November 2026

    bool analyzeFunction(const fnstmt& function);               //done      13th November 2026     
    bool analyzeVariable(const varDeclStmt& variable);          //done      14th November 2026
    bool isNumeric(Type type) const;                            //done      13th November 2026
    bool entryPoint(const program &ast);                        //done      16th November 2026 

    Type resolvetype(const typeinfo& type);                     //done      14th November 2026

    void reportError(
        const std::string& message                              //done      13th November 2026
    );

public:
    bool mainanalyzer(const program& ast);                      //done      14th November 2026
};