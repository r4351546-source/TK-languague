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

