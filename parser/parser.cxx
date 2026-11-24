#include "parser.hxx"
#include "stdexcept"
#include <utility>

parser::parser(std::vector<token>* tokens)
    : tokens(tokens), current(0)
{
    if (this->tokens == nullptr) {
        throw std::invalid_argument(
            "Parser received nullptr"
        );
    }

    if (this->tokens->empty()) {
        throw std::invalid_argument(
            "Parser received empty token vector"
        );
    }

    if (this->tokens->back().TokenType != tokenType::eof) {
        throw std::invalid_argument(
            "Token stream must end with EOF"
        );
    }
}

token parser::previous() {
    return (*tokens)[current - 1];
}

token parser::peek() {
    if (current >= tokens->size()) {
        return tokens->back();
    }

    return (*tokens)[current];
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

ExprPtr parser::call()
{
    ExprPtr expr = primary();

    while (true)
    {
        //простий виклик функції
        if (match(tokenType::leftparen))
        {
            std::vector<ExprPtr> arguments;

            if (!check(tokenType::rightparen))
            {
                do
                {
                    arguments.push_back(expression());
                }
                while (match(tokenType::comma));
            }

            consume(tokenType::rightparen);

            auto call = std::make_unique<callExpr>();

            call->callee = std::move(expr);
            call->arguments = std::move(arguments);

            expr = std::move(call);

            continue;
        }

        //ппостфіксне розіменування p*
        if (isPostfixDereference())
        {
            token op = advance();

            auto unary = std::make_unique<UnaryExpr>();

            unary->op = op;
            unary->right = std::move(expr);

            expr = std::move(unary);

            continue;
        }

        break;
    }

    return expr;
}


ExprPtr parser::unary()
{
    
    if (match(tokenType::New))
    {
        typeinfo allocatedType = parseType();

        
        if (
    allocatedType.baseType.TokenType == tokenType::VOIDTYPE &&
    allocatedType.pointerDepth == 0
    )
    {
    throw std::runtime_error(
        "Cannot allocate an object of type void"
    );
    }

    if (allocatedType.isref)
    {
    throw std::runtime_error(
        "Cannot allocate a reference"
    );
    }

        consume(tokenType::leftparen);

        ExprPtr initializer = expression();

        consume(tokenType::rightparen);

        auto result = std::make_unique<newExpr>();

        result->allocatedType = allocatedType;
        result->initializer = std::move(initializer);

        return result;
    }

    
    if (match(tokenType::Delete))
    {
        ExprPtr operand = unary();

        auto result = std::make_unique<delExpr>();

        result->operand = std::move(operand);

        return result;
    }

    
    if (
        match(tokenType::NOT) ||
        match(tokenType::MINUS) ||
        match(tokenType::PLUS) ||
        match(tokenType::ref) ||
        match(tokenType::MULTIPLY)
    )
    {
        token op = previous();

        ExprPtr operand = unary();

        auto result = std::make_unique<UnaryExpr>();

        result->op = op;
        result->right = std::move(operand);

        return result;
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


bool parser::isTypeStart() {
    return check(tokenType::ptr) ||
           check(tokenType::INTEGERTYPE) ||
           check(tokenType::FLOATTYPE) ||
           check(tokenType::CHARTYPE) ||
           check(tokenType::BOOLTYPE) ||
           check(tokenType::VOIDTYPE);
}



    
typeinfo parser::parseType() {
    typeinfo type;

    while (match(tokenType::ptr)) {
        ++type.pointerDepth;
    }

    if (
        match(tokenType::INTEGERTYPE) ||
        match(tokenType::FLOATTYPE) ||
        match(tokenType::CHARTYPE) ||
        match(tokenType::BOOLTYPE) ||
        match(tokenType::VOIDTYPE)
    ) {
        type.baseType = previous();
    }
    else {
        throw std::runtime_error("Expected type");
    }

    if (match(tokenType::ref)) {
        type.isref = true;
    }

    return type;
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


bool parser::isAssignmentTarget(
    const Expr& expression
) const
{
    //звичайна змінна: x = 10
    if (dynamic_cast<const VariableExpr*>(&expression))
        return true;

    //розіменування: *p = 10 або p* = 10
    if (const auto* unary =
        dynamic_cast<const UnaryExpr*>(&expression))
    {
        return unary->op.TokenType == tokenType::MULTIPLY;
    }

    //дужки не повинні змінювати можливість присвоєння
    //наприклад (*p) = 10
    if (const auto* grouping =
        dynamic_cast<const groupingExpr*>(&expression))
    {
        return grouping->expression &&
               isAssignmentTarget(*grouping->expression);
    }

    return false;
}



ExprPtr parser::assignment()
{
    //спочатку парсимо вираз який може стати ціллю
    ExprPtr expr = logicalOr();

    //перевіряємо чи зустріли оператор присвоєння
    if (
        match(tokenType::ASSIGN) ||
        match(tokenType::PLUSASSIGN) ||
        match(tokenType::MINUSASSIGN) ||
        match(tokenType::MULTIPLYASSIGN) ||
        match(tokenType::DIVIDEASSIGN)
    )
    {
        token op = previous();

        //перевіряємо ліву частину
        if (!isAssignmentTarget(*expr))
        {
            throw std::runtime_error(
                "Invalid assignment target"
            );
        }

        //екурсивно парсимо праву частину
        ExprPtr value = assignment();

        //створюємо вузол AST
        auto result = std::make_unique<assignmentExpr>();

        result->target = std::move(expr);
        result->op = op;
  
        result->value = std::move(value);

        return result;
    }

    return expr;
}

ExprPtr parser::expression() {
    return assignment();
}

ExprPtr parser::primary() {
    
    if (match(tokenType::TRUE) ||
    match(tokenType::FALSE)) {

    auto expr = std::make_unique<LiteralExpr>();

    expr->value = previous();

    return expr;
    }
    
    if (match(tokenType::INTEGER_LITERAL) ||
        match(tokenType::FLOAT_LITERAL)
        || match(tokenType::STRING_LITERAL)
        || match(tokenType::CHAR_LITERAL)
        || match(tokenType::NULLPTR)
        )
     {

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

ExprPtr parser::logicalAnd()
{
    auto expr = equality();

    while (match(tokenType::AND))
    {
        token op = previous();
        auto right = equality();

        auto binary = std::make_unique<BinaryExpr>();

    binary->left = std::move(expr);
    binary->op = op;
    binary->right = std::move(right);

    expr = std::move(binary);
    }

    return expr;
}

ExprPtr parser::logicalOr()
{
    auto expr = logicalAnd();

    while (match(tokenType::OR))
    {
        token op = previous();
        auto right = logicalAnd();

        auto binary = std::make_unique<BinaryExpr>();

        binary->left = std::move(expr);
        binary->op = op;
        binary->right = std::move(right);

        expr = std::move(binary);
    }

    return expr;
}



/*
— end of expression parsing functions —
*/

/*======================================*/

/*
— start of statement parsing functions —
*/


stmtptr parser::expressionStatement() {
    ExprPtr expr = expression();

    consume(tokenType::semicolon);

    auto stmt = std::make_unique<expressionstmt>();
    stmt->expression = std::move(expr);

    return stmt;
}


stmtptr parser::returnStatement() {
    ExprPtr value = nullptr;

    if (!check(tokenType::semicolon)) {
        value = expression();
    }

    consume(tokenType::semicolon);

    auto stmt = std::make_unique<returnstmt>();
    stmt->value = std::move(value);

    return stmt;
}

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
        if (check(tokenType::Const) ||
            isTypeStart()
    ) {

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

    typeinfo returnType;

    if (!isTypeStart()) {
    throw std::runtime_error("Expected return type");
    }

    returnType = parseType();
    
   
    token name = consume(tokenType::IDENTIFIER);

    consume(tokenType::leftparen);


    std::vector<parameters> Parameters;

    if (!check(tokenType::rightparen)) {

        do {

            typeinfo paramType = parseType();

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
    bool isconstant = match(tokenType::Const);

    typeinfo type = parseType();
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
    varDecl->isconst = isconstant;

    return varDecl;
}



stmtptr parser::declaration()
{
    // SourceLoader must expand all file directives.
    if (
        check(tokenType::USE) ||
        check(tokenType::INC)
    )
    {
        throw std::runtime_error(
            "Unexpanded file directive. "
            "Use SourceLoader::load() before parsing."
        );
    }

    if (isHeapAllocationDeclaration())
    {
        advance();

        return heapAllocationDeclaration();
    }

    if (match(tokenType::FNTYPE))
    {
        return functionDeclaration();
    }

    if (
        isTypeStart() ||
        check(tokenType::Const)
    )
    {
        return variableDeclaration();
    }

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




program parser::mainparse() {
    current = 0;
    
    program result;

    while (!isAtEnd()) {
        std::size_t start = current;

        result.statements.push_back(
            declaration()
        );

        if (current == start) {
            throw std::runtime_error(
                "Parser made no progress"
            );
        }
    }

    return result;
}