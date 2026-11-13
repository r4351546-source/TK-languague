#include "analyzer.hxx"
#include <iostream>

void analyzer::reportError(const std::string& message)
{
    std::cerr << "Semantic error: " << message << std::endl;
    errorCount++;
}


bool analyzer::isNumeric(Type type) const
{
    switch (type.kind)
    {
        case typekind::INTEGER:
        case typekind::FLOAT:
            return true;

        default:
            return false;
    }
}

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


Type analyzer::analyzeExpression(const Expr& expression)
{
    const Type invalid{ typekind::INVALID };

    
    if (const auto* literal =
        dynamic_cast<const LiteralExpr*>(&expression))
    {
        switch (literal->value.TokenType)
        {
            case tokenType::INTEGER_LITERAL:
                return Type{ typekind::INTEGER };

            case tokenType::FLOAT_LITERAL:
                return Type{ typekind::FLOAT };

            case tokenType::TRUE:
            case tokenType::FALSE:
                return Type{ typekind::BOOL };

            default:
                reportError("unsupported literal type");
                return invalid;
        }
    }

    
    if (const auto* variable =
        dynamic_cast<const VariableExpr*>(&expression))
    {
        const std::string name(variable->name.lexeme);

        symbol* found = symbols.lookup(name);

        if (!found)
        {
            reportError("undefined variable: " + name);
            return invalid;
        }

        if (found->kind == symbolKind::FUNCTION)
        {
            reportError(
                "function cannot be used as a variable: " + name
            );

            return invalid;
        }

        return found->type;
    }

    // 3. groupingExpr
    if (const auto* grouping =
        dynamic_cast<const groupingExpr*>(&expression))
    {
        if (!grouping->expression)
        {
            reportError("empty grouping expression");
            return invalid;
        }

        return analyzeExpression(*grouping->expression);
    }

    
    if (const auto* unary =
        dynamic_cast<const UnaryExpr*>(&expression))
    {
        if (!unary->right)
        {
            reportError("unary expression has no operand");
            return invalid;
        }

        Type operand = analyzeExpression(*unary->right);

        if (operand.kind == typekind::INVALID)
            return invalid;

        switch (unary->op.TokenType)
        {
            case tokenType::MINUS:
            {
                if (operand.kind == typekind::INTEGER ||
                    operand.kind == typekind::FLOAT)
                {
                    return operand;
                }

                reportError(
                    "Unary '-' requires a numeric operand"
                );

                return invalid;
            }

            case tokenType::NOT:
            {
                if (operand.kind == typekind::BOOL)
                    return Type{ typekind::BOOL };

                reportError(
                    "Logical '!' requires a bool operand"
                );

                return invalid;
            }

            default:
                reportError("Unknown unary operator");
                return invalid;
        }
    }

    
    if (const auto* binary =
        dynamic_cast<const BinaryExpr*>(&expression))
    {
        if (!binary->left || !binary->right)
        {
            reportError("Binary expression has missing operand");
            return invalid;
        }

        Type left = analyzeExpression(*binary->left);
        Type right = analyzeExpression(*binary->right);

        if (left.kind == typekind::INVALID ||
            right.kind == typekind::INVALID)
        {
            return invalid;
        }

        const bool leftNumeric =
            left.kind == typekind::INTEGER ||
            left.kind == typekind::FLOAT;

        const bool rightNumeric =
            right.kind == typekind::INTEGER ||
            right.kind == typekind::FLOAT;

        switch (binary->op.TokenType)
        {
            
            case tokenType::PLUS:
            case tokenType::MINUS:
            case tokenType::MULTIPLY:
            case tokenType::DIVIDE:
            {
                if (!leftNumeric || !rightNumeric)
                {
                    reportError(
                        "Arithmetic operator requires numeric operands"
                    );

                    return invalid;
                }

                // INTEGER + FLOAT -> FLOAT
                if (left.kind == typekind::FLOAT ||
                    right.kind == typekind::FLOAT)
                {
                    return Type{ typekind::FLOAT };
                }

                return Type{ typekind::INTEGER };
            }

            
            case tokenType::EQUAL:
            case tokenType::NOTEQUAL:
            {
                if (!tktype::isCompatible(left, right) &&
                    !(leftNumeric && rightNumeric))
                {
                    reportError(
                        "Incompatible types in equality comparison"
                    );

                    return invalid;
                }

                return Type{ typekind::BOOL };
            }

            
            case tokenType::LESS:
            case tokenType::GREATER:
            case tokenType::LESSEQUAL:
            case tokenType::GREATEREQUAL:
            {
                if (!leftNumeric || !rightNumeric)
                {
                    reportError(
                        "Comparison requires numeric operands"
                    );

                    return invalid;
                }

                return Type{ typekind::BOOL };
            }

           
            case tokenType::AND:
            case tokenType::OR:
            {
                if (left.kind != typekind::BOOL ||
                    right.kind != typekind::BOOL)
                {
                    reportError(
                        "Logical operator requires bool operands"
                    );

                    return invalid;
                }

                return Type{ typekind::BOOL };
            }

            default:
                reportError("Unknown binary operator");
                return invalid;
        }
    }

    
    if (const auto* assignment =
        dynamic_cast<const assignmentExpr*>(&expression))
    {
        const std::string name(assignment->name.lexeme);

        symbol* target = symbols.lookup(name);

        if (!target)
        {
            reportError("Undefined variable: " + name);
            return invalid;
        }

        if (target->kind == symbolKind::FUNCTION)
        {
            reportError(
                "Cannot assign to a function: " + name
            );

            return invalid;
        }

        if (!assignment->value)
        {
            reportError("Assignment has no value");
            return invalid;
        }

        Type valueType = analyzeExpression(*assignment->value);

        if (valueType.kind == typekind::INVALID)
            return invalid;

        if (!tktype::isCompatible(target->type, valueType))
        {
            reportError(
                "Assignment type mismatch: expected " +
                std::string(tktype::typeToString(target->type)) +
                ", got " +
                std::string(tktype::typeToString(valueType))
            );

            return invalid;
        }

        return target->type;
    }


    if (const auto* call =
        dynamic_cast<const callExpr*>(&expression))
    {
        const auto* callee =
            dynamic_cast<const VariableExpr*>(call->callee.get());

        if (!callee)
        {
            reportError(
                "function call requires a function identifier"
            );

            return invalid;
        }

        const std::string name(callee->name.lexeme);

        symbol* function = symbols.lookup(name);

        if (!function)
        {
            reportError("undefined function: " + name);
            return invalid;
        }

        if (function->kind != symbolKind::FUNCTION)
        {
            reportError(
                "called object is not a function: " + name
            );

            return invalid;
        }

        bool valid = true;

        if (call->arguments.size() !=
            function->parameterTypes.size())
        {
            reportError(
                "incorrect number of arguments in call to: " + name
            );

            valid = false;
        }

        const size_t count =
            call->arguments.size() < function->parameterTypes.size()
            ? call->arguments.size()
            : function->parameterTypes.size();

        for (size_t i = 0; i < call->arguments.size(); ++i)
        {
            if (!call->arguments[i])
            {
                reportError("Empty function argument");
                valid = false;
                continue;
            }

            Type argumentType =
                analyzeExpression(*call->arguments[i]);

            if (argumentType.kind == typekind::INVALID)
            {
                valid = false;
                continue;
            }

            if (i < count &&
                !tktype::isCompatible(
                    function->parameterTypes[i],
                    argumentType
                ))
            {
                reportError(
                    "function argument type mismatch in: " + name
                );

                valid = false;
            }
        }

        if (!valid)
            return invalid;

        return function->type;
    }
    reportError("unknown expression type");

    return invalid;
}



bool analyzer::analyzeFunction(const fnstmt& function)
{
    bool valid = true;

    const std::string name(function.name.lexeme);

    
    Type returnType = resolvetype(function.returnType);

    if (returnType.kind == typekind::INVALID)
    {
        reportError(
            "Unknown return type in function: " + name
        );

        return false;
    }

    
    if (symbols.lookupcurrent(name))
    {
        reportError(
            "Function already declared: " + name
        );

        return false;
    }

    
    std::vector<Type> parameterTypes;

    for (const auto& parameter : function.parameters)
    {
        Type parameterType = resolvetype(parameter.type);

        if (parameterType.kind == typekind::INVALID ||
            parameterType.kind == typekind::VOID)
        {
            reportError(
                "Invalid parameter type in function: " + name
            );

            valid = false;
            continue;
        }

        parameterTypes.push_back(parameterType);
    }

    
    if (!valid)
        return false;

    
    symbol functionSymbol(
        name,
        returnType,
        parameterTypes
    );

    if (!symbols.declare(functionSymbol))
    {
        reportError(
            "Failed to declare function: " + name
        );

        return false;
    }

    
    Type previousReturnType = currentReturnType;
    bool previousInsideFunction = insideFunction;

    currentReturnType = returnType;
    insideFunction = true;

    
    symbols.enterscope();

    
    for (const auto& parameter : function.parameters)
    {
        const std::string parameterName(
            parameter.name.lexeme
        );

        Type parameterType = resolvetype(parameter.type);

        if (symbols.lookupcurrent(parameterName))
        {
            reportError(
                "Duplicate parameter: " + parameterName
            );

            valid = false;
            continue;
        }

        symbol parameterSymbol(
            parameterName,
            symbolKind::PARAMETER,
            parameterType
        );

        if (!symbols.declare(parameterSymbol))
        {
            reportError(
                "failed to declare parameter: " + parameterName
            );

            valid = false;
        }
    }

    
    for (const auto& statement : function.body)
    {
        if (!statement)
        {
            reportError(
                "null statement in function: " + name
            );

            valid = false;
            continue;
        }

        if (!analyzeStatement(*statement)){
            valid = false;
        }
    }

    
    symbols.exitscope();

    
    currentReturnType = previousReturnType;
    insideFunction = previousInsideFunction;

    return valid;
}


