#include "lexer.hxx"
#include "../file/file.hxx"
#include <cctype>
#include <utility>

char lexer::advance() {
    if(current >= src.size()) {
        return '\0';
    }
    
    char c = src[current++];

    if (c == '\n')
    {
        ++line;
        column = 1;
    }

    else
    {
        ++column;
    }

    return c;
}

char lexer::peek() {
    if(current >= src.size()) {
        return '\0';
    }

    return src[current];
}

void lexer::skipWhitespace() {
    while (
        current < src.size() &&
        std::isspace(
            static_cast<unsigned char>(peek())
        )
    )
    {
        advance();
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
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::ASSIGN,
                std::string(src.data() + start, current - start)};

        case '+':
            if(peek() == '=') {
                advance();
                return {tokenType::PLUSASSIGN,
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::PLUS,
                std::string(src.data() + start, current - start)};

        case '-':
            if(peek() == '=') {
                advance();
                return {tokenType::MINUSASSIGN,
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::MINUS,
                std::string(src.data() + start, current - start)};

        case '*':
            if(peek() == '=') {
                advance();
                return {tokenType::MULTIPLYASSIGN,
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::MULTIPLY,
                std::string(src.data() + start, current - start)};

        case '/':
            if(peek() == '=') {
                advance();
                return {tokenType::DIVIDEASSIGN,
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::DIVIDE,
                std::string(src.data() + start, current - start)};

        // Comparison operators
        case '!':
            if(peek() == '=') {
                advance();
                return {tokenType::NOTEQUAL,
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::NOT,
                std::string(src.data() + start, current - start)};
            break;

        case '<':
            if(peek() == '=') {
                advance();
                return {tokenType::LESSEQUAL,
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::LESS,
                std::string(src.data() + start, current - start)};

        case '>':
            if(peek() == '=') {
                advance();
                return {tokenType::GREATEREQUAL,
                    std::string(src.data() + start, current - start)};
            }
            return {tokenType::GREATER,
                std::string(src.data() + start, current - start)};

        // Punctuation
        case ';':
            return {tokenType::semicolon,
                std::string(src.data() + start, current - start)};
        
        case ':':
            if (match(':')) {
                return {tokenType::scope,
                    std::string(src.data() + start, current - start)};
            }
            break;

        case ',':
            return {tokenType::comma,
                std::string(src.data() + start, current - start)};
                
        case '(':
            return {tokenType::leftparen,
                std::string(src.data() + start, current - start)};

        case ')':
            return {tokenType::rightparen,
                std::string(src.data() + start, current - start)};

        case '{':
            return {tokenType::leftbrace,
                std::string(src.data() + start, current - start)};

        case '}':
            return {tokenType::rightbrace,
                std::string(src.data() + start, current - start)};
        
        case '&':
            if (match('&')) {
            return {tokenType::AND, "&&"};
            }
            return {tokenType::ref,
                std::string(src.data() + start, current - start)
            };

        case '|':
            if (match('|')) {
            return {tokenType::OR, "||"};
            }
            break;
    }

    return {tokenType::invalid,
        std::string(src.data() + start, current - start)};
}


token lexer::readIdentifier() {
    while(
        std::isalnum(static_cast<unsigned char>(peek())) || 
        peek() == '_' 
    ) {
        advance();
    }
    std::string lexeme(
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
    else if(lexeme == "const") {
        type = tokenType::Const;
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
    else if (lexeme == "use") {
        type = tokenType::USE;
    }
    else if (lexeme == "inc") { 
        type = tokenType::INC;
    }
    else if(lexeme == "ptr") {
        type = tokenType::ptr;
    }
    else if(lexeme == "new") {
        type = tokenType::New;
    }
    else if(lexeme == "delete") {
        type = tokenType::Delete;
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

    std::string lexeme(
        src.data() + start,
        current - start
    );

    return {
        isFloat ? tokenType::FLOAT_LITERAL : tokenType::INTEGER_LITERAL,
        lexeme
    };
}


void lexer::addToken(tokenType type, std::string value) {
    tokens.push_back({type, std::string(value)});
}


token lexer::readstr()
{
    advance(); // Opening quote

    bool escaped = false;

    while (current < src.size())
    {
        char c = advance();

        if (escaped)
        {
            escaped = false;
            continue;
        }

        if (c == '\\')
        {
            escaped = true;
            continue;
        }

        if (c == '"')
        {
            return {
                tokenType::STRING_LITERAL,
                src.substr(start, current - start)
            };
        }

        if (c == '\n')
        {
            return {
                tokenType::invalid,
                src.substr(start, current - start)
            };
        }
    }

    return {
        tokenType::invalid,
        src.substr(start, current - start)
    };
}

token lexer::readchar()
{
    advance();              //opening '

    bool valid = true;

    if (
        current >= src.size() ||
        peek() == '\n' ||
        peek() == '\''
    )
    {
        valid = false;
    }
    else
    {
        char value = advance();

        if (value == '\\')
        {
            if (
                current >= src.size() ||
                peek() == '\n'
            )
            {
                valid = false;
            }
            else
            {
                char escaped = advance();

                switch (escaped)
                {
                    case '\'':
                    case '\\':
                    case 'n':
                    case 't':
                    case 'r':
                    case '0':
                        break;

                    default:
                        valid = false;
                        break;
                }
            }
        }
    }

    if (!match('\''))
    {
        valid = false;

        // Consume malformed character literal.
        while (
            current < src.size() &&
            peek() != '\n' &&
            peek() != '\''
        )
        {
            advance();
        }

        if (peek() == '\'')
        {
            advance();
        }
    }

    return {
        valid
            ? tokenType::CHAR_LITERAL
            : tokenType::invalid,

        src.substr(start, current - start)
    };
}

std::vector<token> lexer::mainlexer(const std::string& source)
{
    src = source;

    tokens.clear();

    current = 0;
    start = 0;

    line = 1;
    column = 1;

    while (current < src.size())
    {
        start = current;

        // Comments
        if (
            peek() == '/' &&
            current + 1 < src.size() &&
            src[current + 1] == '/'
        )
        {
            skipcomment();
            continue;
        }

        // Whitespace
        if (std::isspace(
            static_cast<unsigned char>(peek())
        ))
        {
            advance();
            skipWhitespace();
            continue;
        }

        // Identifiers and keywords
        if (
            std::isalpha(
                static_cast<unsigned char>(peek())
            ) ||
            peek() == '_'
        )
        {
            token value = readIdentifier();

            addToken(
                value.TokenType,
                std::move(value.lexeme)
            );

            continue;
        }

        // Numbers
        if (
            std::isdigit(
                static_cast<unsigned char>(peek())
            )
        )
        {
            token value = readNumber();

            addToken(
                value.TokenType,
                std::move(value.lexeme)
            );

            continue;
        }

        // Strings
        if (peek() == '"')
        {
            token value = readstr();

            addToken(
                value.TokenType,
                std::move(value.lexeme)
            );

            continue;
        }

        // Character literals
        if (peek() == '\'')
        {
        token value = readchar();

            addToken(
            value.TokenType,
            std::move(value.lexeme)
            );

        continue;
        }

        // Operators and punctuation
        token value = readOperator(peek());

        addToken(
            value.TokenType,
            std::move(value.lexeme)
        );
    }

    addToken(tokenType::eof, "");

    return tokens;
}


std::vector<token> lexer::getFileLexer(const std::string& path) {
    file f;
    std::string source = f.readFile(path);
    return mainlexer(source);
}


