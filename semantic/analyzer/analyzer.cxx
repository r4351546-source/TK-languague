#include "analyzer.hxx"

bool analyzer::analyzeStatement(const stmt& statement)
{
    if (const auto* variable =
        dynamic_cast<const varDeclStmt*>(&statement))
    {
        return analyzeVariable(*variable);
    }

    
    if (const auto* function =
        dynamic_cast<const fnstmt*>(&statement))
    {
        return analyzeFunction(*function);
    }

    
    if (const auto* block =
        dynamic_cast<const Blockstmt*>(&statement))
    {
        symbols.enterscope();

        bool valid = true;

        for (const auto& statement : block->statements)
        {
            if (!analyzeStatement(*statement))
                valid = false;
        }

        symbols.exitscope();

        return valid;
    }

    
    if (const auto* ifNode =
        dynamic_cast<const Ifstmt*>(&statement))
    {
        bool valid = true;

        if (!ifNode->condition)
        {
            reportError("If statement has no condition");
            return false;
        }

        Type conditionType =
            analyzeExpression(*ifNode->condition);

        if (conditionType.kind != typekind::BOOL)
        {
            reportError("If condition must be bool");
            valid = false;
        }

        if (ifNode->thenBranch)
        {
            if (!analyzeStatement(*ifNode->thenBranch))
                valid = false;
        }

        if (ifNode->elseBranch)
        {
            if (!analyzeStatement(*ifNode->elseBranch))
                valid = false;
        }

        return valid;
    }

    
    if (const auto* whileNode =
        dynamic_cast<const whileStmt*>(&statement))
    {
        bool valid = true;

        if (!whileNode->condition)
        {
            reportError("While statement has no condition");
            return false;
        }

        Type conditionType =
            analyzeExpression(*whileNode->condition);

        if (conditionType.kind != typekind::BOOL)
        {
            reportError("While condition must be bool");
            valid = false;
        }

        if (whileNode->body)
        {
            if (!analyzeStatement(*whileNode->body))
                valid = false;
        }

        return valid;
    }

   
    if (const auto* forNode =
        dynamic_cast<const forStmt*>(&statement))
    {
        bool valid = true;

        symbols.enterscope();

        
        if (forNode->initializer)
        {
            if (!analyzeStatement(*forNode->initializer))
                valid = false;
        }

        
        if (forNode->condition)
        {
            Type conditionType =
                analyzeExpression(*forNode->condition);

            if (conditionType.kind != typekind::BOOL)
            {
                reportError("For condition must be bool");
                valid = false;
            }
        }

        
        if (forNode->increment)
        {
            Type incrementType =
                analyzeExpression(*forNode->increment);

            if (incrementType.kind == typekind::INVALID)
                valid = false;
        }

        
        if (forNode->body)
        {
            if (!analyzeStatement(*forNode->body))
                valid = false;
        }

        symbols.exitscope();

        return valid;
    }

    
    if (const auto* expression =
        dynamic_cast<const expressionstmt*>(&statement))
    {
        if (!expression->expression)
        {
            reportError("Empty expression statement");
            return false;
        }

        Type expressionType =
            analyzeExpression(*expression->expression);

        return expressionType.kind != typekind::INVALID;
    }

    
    if (const auto* returnNode =
        dynamic_cast<const returnstmt*>(&statement))
    {
        
        if (currentReturnType.kind == typekind::INVALID)
        {
            reportError("return statement outside function");
            return false;
        }

        
        if (!returnNode->value)
        {
            if (currentReturnType.kind != typekind::VOID)
            {
                reportError(
                    "not void function must return a value"
                );

                return false;
            }

            return true;
        }

        Type valueType =
            analyzeExpression(*returnNode->value);

        if (valueType.kind == typekind::INVALID)
            return false;

        if (currentReturnType.kind == typekind::VOID)
        {
            reportError(
                "void function cannot return a value"
            );

            return false;
        }

        if (!tktype::isCompatible(currentReturnType, valueType))
        {
            reportError(
                "return type mismatch: expected " +
                std::string(tktype::typeToString(currentReturnType)) +
                ", got " +
                std::string(tktype::typeToString(valueType))
            );

            return false;
        }

        return true;
    }

    reportError("unknown statement type");

    return false;
}

Type analyzer::analyzeExpression(const Expr& expression) {

}