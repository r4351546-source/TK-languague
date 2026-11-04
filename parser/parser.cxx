#include "parser.hxx"

bool parser::isAtEnd() {
    return peek().TokenType == tokenType::eof;
}

token parser::previous() {
    return tokens[current - 1];
}

token parser::peek() {
    return tokens[current];
}

token parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}