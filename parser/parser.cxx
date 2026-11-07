#include "parser.hxx"
#include "stdexcept"
#include <utility>


token parser::previous() {
    return tokens[current - 1];
}

token parser::peek() {
    return tokens[current];
}

bool parser::isAtEnd() {
    return peek().TokenType == tokenType::eof;
}

token parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool parser::check(tokenType type) {
    if (isAtEnd()) return false;
    return peek().TokenType == type;
}

bool parser::match(tokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

token parser::consume(tokenType type) {
    if (check(type)) return advance();

    throw std::runtime_error("unexpected token");
}

ExprPtr parser::call() {
    ExprPtr expr = primary();

    while (match(tokenType::leftparen)) {
        std::vector<ExprPtr> arguments;

        if (!check(tokenType::rightparen)) {
            do {
                arguments.push_back(expression());
            } while (match(tokenType::comma));
        }

        consume(tokenType::rightparen);

        auto CallExpr = std::make_unique<callExpr>();
        CallExpr->callee = std::move(expr);
        CallExpr->arguments = std::move(arguments);

        expr = std::move(CallExpr);
    }

    return expr;
}

ExprPtr parser::unary() {
    if (match(tokenType::MINUS) ||
        match(tokenType::PLUS)) {

        token op = previous();
        ExprPtr operand = unary();

        auto expr = std::make_unique<UnaryExpr>();
        expr->op = op;
        expr->right = std::move(operand);

        return expr;
    }

    return call();
}

ExprPtr parser::factor() {
    ExprPtr expr = unary();

    while (match(tokenType::MULTIPLY) || match(tokenType::DIVIDE)) {
        token op = previous();
        ExprPtr right = unary();

        auto binary = std::make_unique<BinaryExpr>();

        binary->left = std::move(expr);
        binary->op = op;
        binary->right = std::move(right);

        expr = std::move(binary);
    }

    return expr;
}


ExprPtr parser::term() {
    ExprPtr expr = factor();

    while (match(tokenType::PLUS) || match(tokenType::MINUS)) {
        token op = previous();
        ExprPtr right = factor();

        auto binary = std::make_unique<BinaryExpr>();

        binary->left = std::move(expr);
        binary->op = op;
        binary->right = std::move(right);

        expr = std::move(binary);
    }

    return expr;


}
ExprPtr parser::comparison() {
    ExprPtr expr = term();

    while (match(tokenType::GREATER) ||
           match(tokenType::GREATEREQUAL) ||
           match(tokenType::LESS) ||
           match(tokenType::LESSEQUAL)) {

        token op = previous();
        ExprPtr right = term();

        auto binary = std::make_unique<BinaryExpr>();

        binary->left = std::move(expr);
        binary->op = op;
        binary->right = std::move(right);

        expr = std::move(binary);
    }

    return expr;
}

ExprPtr parser::equality() {
    ExprPtr expr = comparison();

    while (match(tokenType::EQUAL) ||
           match(tokenType::NOTEQUAL)) {

        token op = previous();
        ExprPtr right = comparison();

        auto binary = std::make_unique<BinaryExpr>();

        binary->left = std::move(expr);
        binary->op = op;
        binary->right = std::move(right);

        expr = std::move(binary);
    }

    return expr;
}


ExprPtr parser::assignment() {
    ExprPtr expr = equality();

    if (match(tokenType::ASSIGN)) {
        token equals = previous();
        ExprPtr value = assignment();

        if (auto varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            auto assignExpr = std::make_unique<assignmentExpr>();
            assignExpr->name = varExpr->name;
            assignExpr->value = std::move(value);
            return assignExpr;
        }

        throw std::runtime_error("Invalid assignment target");
    }

    return expr;
}

ExprPtr parser::expression() {
    return assignment();
}

ExprPtr parser::primary() {
    if (match(tokenType::INTEGER_LITERAL) ||
        match(tokenType::FLOAT_LITERAL)) {

        auto expr = std::make_unique<LiteralExpr>();
        expr->value = previous();

        return expr;
    }

    if (match(tokenType::IDENTIFIER)) {
        auto expr = std::make_unique<VariableExpr>();
        expr->name = previous();

        return expr;
    }

    if (match(tokenType::leftparen)) {
        ExprPtr expr = expression();

        consume(tokenType::rightparen);

        auto group = std::make_unique<groupingExpr>();
        group->expression = std::move(expr);

        return group;
    }

    throw std::runtime_error("Expected expression");
}

/*
— end of expression parsing functions —
*/


/*
— start of statement parsing functions —
*/

stmtptr parser::whileStatement() {
    consume(tokenType::leftparen);

    ExprPtr condition = expression();

    consume(tokenType::rightparen);

    stmtptr body = statement();

    auto whiles = std::make_unique<whileStmt>();

    whiles->condition = std::move(condition);
    whiles->body = std::move(body);

    return whiles;
}

stmtptr parser::ifStatement() {
    consume(tokenType::leftparen);

    ExprPtr condition = expression();

    consume(tokenType::rightparen);

    stmtptr thenBranch = statement();

    stmtptr elseBranch = nullptr;

    if (match(tokenType::ELSE)) {
        elseBranch = statement();
    }

    auto stmt = std::make_unique<Ifstmt>();

    stmt->condition = std::move(condition);
    stmt->thenBranch = std::move(thenBranch);
    stmt->elseBranch = std::move(elseBranch);

    return stmt;
}

stmtptr parser::forStatement() {
    consume(tokenType::leftparen);

    stmtptr initializer = nullptr;

    if (!check(tokenType::semicolon)) {
        if (match(tokenType::INTEGERTYPE) ||
            match(tokenType::FLOATTYPE) ||
            match(tokenType::CHARTYPE) ||
            match(tokenType::BOOLTYPE)) {

            initializer = variableDeclaration();

        }
        
        else {
            ExprPtr expr = expression();

            consume(tokenType::semicolon);

            auto expressionStmt = std::make_unique<expressionstmt>();
            expressionStmt->expression = std::move(expr);

            initializer = std::move(expressionStmt);
        }
    } 
    else {
        consume(tokenType::semicolon);
    }

    ExprPtr condition = nullptr;

    if (!check(tokenType::semicolon)) {
        condition = expression();
    }

    consume(tokenType::semicolon);

    ExprPtr increment = nullptr;

    if (!check(tokenType::rightparen)) {
        increment = expression();
    }

    consume(tokenType::rightparen);

    stmtptr body = statement();

    auto loop = std::make_unique<forStmt>();

    loop->initializer = std::move(initializer);
    loop->condition = std::move(condition);
    loop->increment = std::move(increment);
    loop->body = std::move(body);

    return loop;
}



stmtptr parser::statement() {
    if (match(tokenType::IF)) return ifStatement();
    if (match(tokenType::WHILE)) return whileStatement();
    if (match(tokenType::FOR)) return forStatement();
    if (match(tokenType::Return)) return returnStatement();
    if (match(tokenType::leftbrace)) return blockStatement();

    return expressionStatement();
}

stmtptr parser::functionDeclaration() {

    token returnType;

    if (match(tokenType::INTEGERTYPE) ||
        match(tokenType::FLOATTYPE) ||
        match(tokenType::CHARTYPE) ||
        match(tokenType::BOOLTYPE)
        || match(tokenType::VOIDTYPE)
    ) {

        returnType = previous();

    } 
    
    else {
        throw std::runtime_error("Expected return type");
    }


    token name = consume(tokenType::IDENTIFIER);

    consume(tokenType::leftparen);


    std::vector<parameters> Parameters;

    if (!check(tokenType::rightparen)) {

        do {

            token paramType;

            if (match(tokenType::INTEGERTYPE) ||
                match(tokenType::FLOATTYPE) ||
                match(tokenType::CHARTYPE) ||
                match(tokenType::BOOLTYPE)
                || match(tokenType::VOIDTYPE)
            ) {

                paramType = previous();

            } else {
                throw std::runtime_error("Expected parameter type");
            }

            token paramName = consume(tokenType::IDENTIFIER);

            Parameters.push_back({
                paramType,
                paramName
            });

        } while (match(tokenType::comma));
    }

    consume(tokenType::rightparen);
    consume(tokenType::leftbrace);

    std::vector<stmtptr> body;

    while (!check(tokenType::rightbrace) && !isAtEnd()) {
        body.push_back(declaration());
    }

    consume(tokenType::rightbrace);

    auto function = std::make_unique<fnstmt>();

    function->name = name;
    function->returnType = returnType;
    function->parameters = std::move(Parameters);
    function->body = std::move(body);

    return function;
}


stmtptr parser::variableDeclaration() {
    token type = previous();
    token name = consume(tokenType::IDENTIFIER);

    ExprPtr initializer = nullptr;

    if (match(tokenType::ASSIGN)) {
        initializer = expression();
    }

    consume(tokenType::semicolon);

    auto varDecl = std::make_unique<varDeclStmt>();
    varDecl->type = type;
    varDecl->name = name;
    varDecl->initializer = std::move(initializer);

    return varDecl;
}



stmtptr parser::declaration() {
    if (match(tokenType::FNTYPE)) return functionDeclaration();
    if (match(tokenType::INTEGERTYPE) || match(tokenType::FLOATTYPE) || match(tokenType::CHARTYPE) || match(tokenType::BOOLTYPE)) return variableDeclaration();

    return statement();
}

stmtptr parser::blockStatement() {
    std::vector<stmtptr> statements;

    while (!check(tokenType::rightbrace) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    consume(tokenType::rightbrace);

    auto block = std::make_unique<Blockstmt>();
    block->statements = std::move(statements);

    return block;
}


