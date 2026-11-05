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

bool parser::match(tokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool parser::check(tokenType type) {
    if (isAtEnd()) return false;
    return peek().TokenType == type;
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

