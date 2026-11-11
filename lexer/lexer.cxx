#include "lexer.hxx"
#include "../file/file.hxx"
#include <cctype>

char lexer::advance() {
    return src[current++];
}

char lexer::peek() {
    if(current >= src.size()) {
        return '\0';
    }

    return src[current];
}

void lexer::skipWhitespace() {
    while(current < src.size() && 
    std::isspace(static_cast<unsigned char>(src[current]))) {
        current++;
    }
}

void lexer::skipcomment() {
    if (peek() != '/' || current + 1 >= src.size() ||
        src[current + 1] != '/')
        return;

    while (peek() != '\n' && peek() != '\0')
        advance();
}

token lexer::readOperator(char op) {

    op = advance();

    switch(op) {

        // Assignment operators
        case '=':
            if(peek() == '=') {
                advance();
                return {tokenType::EQUAL, 
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::ASSIGN,
                std::string_view(src.data() + start, current - start)};

        case '+':
            if(peek() == '=') {
                advance();
                return {tokenType::PLUSASSIGN,
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::PLUS,
                std::string_view(src.data() + start, current - start)};

        case '-':
            if(peek() == '=') {
                advance();
                return {tokenType::MINUSASSIGN,
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::MINUS,
                std::string_view(src.data() + start, current - start)};

        case '*':
            if(peek() == '=') {
                advance();
                return {tokenType::MULTIPLYASSIGN,
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::MULTIPLY,
                std::string_view(src.data() + start, current - start)};

        case '/':
            if(peek() == '=') {
                advance();
                return {tokenType::DIVIDEASSIGN,
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::DIVIDE,
                std::string_view(src.data() + start, current - start)};

        // Comparison operators
        case '!':
            if(peek() == '=') {
                advance();
                return {tokenType::NOTEQUAL,
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::NOT,
                std::string_view(src.data() + start, current - start)};
            break;

        case '<':
            if(peek() == '=') {
                advance();
                return {tokenType::LESSEQUAL,
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::LESS,
                std::string_view(src.data() + start, current - start)};

        case '>':
            if(peek() == '=') {
                advance();
                return {tokenType::GREATEREQUAL,
                    std::string_view(src.data() + start, current - start)};
            }
            return {tokenType::GREATER,
                std::string_view(src.data() + start, current - start)};

        // Punctuation
        case ';':
            return {tokenType::semicolon,
                std::string_view(src.data() + start, current - start)};

        case ',':
            return {tokenType::comma,
                std::string_view(src.data() + start, current - start)};
                
        case '(':
            return {tokenType::leftparen,
                std::string_view(src.data() + start, current - start)};

        case ')':
            return {tokenType::rightparen,
                std::string_view(src.data() + start, current - start)};

        case '{':
            return {tokenType::leftbrace,
                std::string_view(src.data() + start, current - start)};

        case '}':
            return {tokenType::rightbrace,
                std::string_view(src.data() + start, current - start)};
        
        case '&':
            if (match('&'))
            return {tokenType::AND, "&&"};
            break;

        case '|':
            if (match('|'))
            return {tokenType::OR, "||"};
            break;
    }

    return {tokenType::invalid,
        std::string_view(src.data() + start, current - start)};
}


token lexer::readIdentifier() {
    while(
        std::isalnum(static_cast<unsigned char>(peek())) || 
        peek() == '_' 
    ) {
        advance();
    }
    std::string_view lexeme(
        src.data() + start, current - start
    );
    tokenType type = tokenType::IDENTIFIER;
    if(lexeme == "if") {
        type = tokenType::IF;
    }
    else if(lexeme == "for") {
        type = tokenType::FOR;
    }
    else if(lexeme == "else") {
        type = tokenType::ELSE;
    }
    else if(lexeme == "while") {
        type = tokenType::WHILE;
    }
    else if(lexeme == "int") {
        type = tokenType::INTEGERTYPE;
    }
    else if(lexeme == "char") {
        type = tokenType::CHARTYPE;
    }
    else if(lexeme == "float") {
        type = tokenType::FLOATTYPE;
    }
    else if(lexeme == "return") {
        type = tokenType::Return;
    }
    else if(lexeme == "bool") {
        type = tokenType::BOOLTYPE;
    }
    else if(lexeme == "fn") {
        type = tokenType::FNTYPE;
    }
    else if (lexeme == "void") {
    type = tokenType::VOIDTYPE;
    }
    else if (lexeme == "true") {
        type = tokenType::TRUE;
    }
    else if (lexeme == "false") {
        type = tokenType::FALSE;
    }
    
        return {type, lexeme};
}

bool lexer::match(char expect) {
     if (current >= src.size()) {
        return false;
    }

    if (peek() != expect) {
        return false;
    }

    advance();
    return true;
}

token lexer::readNumber() {
    bool isFloat = false;

    while(std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if(peek() == '.' &&
       current + 1 < src.size() &&
       std::isdigit(static_cast<unsigned char>(src[current + 1]))) {

        isFloat = true;
        advance();

        while(std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    std::string_view lexeme(
        src.data() + start,
        current - start
    );

    return {
        isFloat ? tokenType::FLOAT_LITERAL : tokenType::INTEGER_LITERAL,
        lexeme
    };
}


void lexer::addToken(tokenType type, std::string_view value) {
    tokens.push_back({type, value});
}


std::vector<token> lexer::mainlexer(const std::string& source) {
    src = source;
    tokens.clear();
    current = 0;
    start = 0;

    while (current < src.size()) {
    start = current;

    if (std::isspace(static_cast<unsigned char>(peek()))) {
        advance();
        skipWhitespace();
        continue;
    }

    if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
        token identifierToken = readIdentifier();
        addToken(identifierToken.TokenType, identifierToken.lexeme);
        continue;
    }

    if (std::isdigit(static_cast<unsigned char>(peek()))) {
        token numberToken = readNumber();
        addToken(numberToken.TokenType, numberToken.lexeme);
        continue;
    }

    token operatorToken = readOperator(peek());

    if (operatorToken.TokenType != tokenType::invalid) {
        addToken(operatorToken.TokenType, operatorToken.lexeme);
        continue;
    }

    addToken(tokenType::invalid,
        std::string_view(src.data() + start, current - start));
    }
    
    addToken(tokenType::eof, std::string_view());
    return tokens;
}


std::vector<token> lexer::getFileLexer(const std::string& path) {
    std::string source = file::readFile(path);
    return mainlexer(source);
}