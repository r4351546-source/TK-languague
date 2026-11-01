#pragma once
#include <string_view>
#include <vector>
#include <string>

enum class tokenType {
    IDENTIFIER,

    //awarding
    ASSIGN,     //=
    PLUSASSIGN, //+=
    MINUSASSIGN,//-=
    MULTIPLYASSIGN,//*=
    DIVIDEASSIGN,  // /=

    //arithmetic
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,


    //simile
    EQUAL,      //==
    NOTEQUAL,   //!=
    LESS,       //<
    GREATER,    //>
    LESSEQUAL,  //<=
    GREATEREQUAL,//>=  

    //variables
    INTEGER,
    FLOAT,
    CHAR,
    BOOL,
    
    //conditions
    IF,
    ELIF,
    ELSE,

    //other
    semicolon,   //semicolon — ;
    leftparen,   //leftparen — (
    rightparen,  //rightparen — )
    leftbrace,   //leftbrace — {
    rightbrace,   //rightbrace — }
    invalid,
    eof
};

struct token {
    tokenType TokenType;
    std::string_view lexeme;
};

class lexer {
    
    private:
    
    std::string src;
    std::vector<token> tokens;
    char advance(); //переміщує на наступний символ
    char peek();    //дивиться на символ без переміщення
    void skipWhitespace(); 
    void skipcomment();
    bool match(char expect);
    token readIdentifier();
    token readNumber();
    token readOperator();

    int line = 1;
    int column = 1;
    std::size_t current = 0;
    std::size_t start = 0;    

    void addToken(tokenType type, std::string_view value);

    public:
    std::vector<token> mainlexer(const std::string& source);
    std::vector<token> getFileLexer(const std::string& path);
   
};
