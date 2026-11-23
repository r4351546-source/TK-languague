#include "lexer.hxx"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct expected_token {
    tokenType type;
    std::string lexeme;
};

static std::size_t passed = 0;
static std::size_t failed = 0;

void expectTokens(
    lexer& scanner,
    const std::string& testName,
    const std::string& source,
    std::vector<expected_token> expected
) {
    
    expected.push_back({tokenType::eof, ""});

    const std::vector<token> actual = scanner.mainlexer(source);
    bool ok = true;

    if (actual.size() != expected.size()) {
        std::cerr << "[FAIL] " << testName
                  << ": expected " << expected.size()
                  << " tokens, got " << actual.size() << '\n';
        ok = false;
    }

    const std::size_t count =
        actual.size() < expected.size() ? actual.size() : expected.size();

    for (std::size_t i = 0; i < count; ++i) {
        if (actual[i].TokenType != expected[i].type ||
            actual[i].lexeme != expected[i].lexeme) {
            std::cerr << "  token[" << i << "]: expected type="
                      << static_cast<int>(expected[i].type)
                      << ", lexeme=\"" << expected[i].lexeme
                      << "\"; got type="
                      << static_cast<int>(actual[i].TokenType)
                      << ", lexeme=\"" << actual[i].lexeme << "\"\n";
            ok = false;
        }
    }

    if(ok) {
        ++passed;
        std::cout << "[PASS] " << testName << '\n';
    } else {
        ++failed;
    }
}

int main() {
    lexer scanner;

    
    expectTokens(scanner, "keywords",
        "if else while for int float char bool fn void return "
        "true false nullptr use inc const ptr new delete",
        {
            {tokenType::IF, "if"},
            {tokenType::ELSE, "else"},
            {tokenType::WHILE, "while"},
            {tokenType::FOR, "for"},
            {tokenType::INTEGERTYPE, "int"},
            {tokenType::FLOATTYPE, "float"},
            {tokenType::CHARTYPE, "char"},
            {tokenType::BOOLTYPE, "bool"},
            {tokenType::FNTYPE, "fn"},
            {tokenType::VOIDTYPE, "void"},
            {tokenType::Return, "return"},
            {tokenType::TRUE, "true"},
            {tokenType::FALSE, "false"},
            {tokenType::NULLPTR, "nullptr"},
            {tokenType::USE, "use"},
            {tokenType::INC, "inc"},
            {tokenType::Const, "const"},
            {tokenType::ptr, "ptr"},
            {tokenType::New, "new"},
            {tokenType::Delete, "delete"}
        }
    );

    
    expectTokens(scanner, "identifiers",
        "tre da tas linux os_Sss9",
        {
            {tokenType::IDENTIFIER, "tre"},
            {tokenType::IDENTIFIER, "da"},
            {tokenType::IDENTIFIER, "tas"},
            {tokenType::IDENTIFIER, "linux"},
            {tokenType::IDENTIFIER, "os_Sss9"}
        }
    );

    
    expectTokens(scanner, "numeric literals",
        "0 17 001 3.14 0.5 123.456",
        {
            {tokenType::INTEGER_LITERAL, "0"},
            {tokenType::INTEGER_LITERAL, "17"},
            {tokenType::INTEGER_LITERAL, "001"},
            {tokenType::FLOAT_LITERAL, "3.14"},
            {tokenType::FLOAT_LITERAL, "0.5"},
            {tokenType::FLOAT_LITERAL, "123.456"}
        }
    );

    
    expectTokens(scanner, "operators and punctuation",
        "= == += -= *= /= + - * / ! != < <= > >= && || & :: ; , ( ) { }",
        {
            {tokenType::ASSIGN, "="},
            {tokenType::EQUAL, "=="},
            {tokenType::PLUSASSIGN, "+="},
            {tokenType::MINUSASSIGN, "-="},
            {tokenType::MULTIPLYASSIGN, "*="},
            {tokenType::DIVIDEASSIGN, "/="},
            {tokenType::PLUS, "+"},
            {tokenType::MINUS, "-"},
            {tokenType::MULTIPLY, "*"},
            {tokenType::DIVIDE, "/"},
            {tokenType::NOT, "!"},
            {tokenType::NOTEQUAL, "!="},
            {tokenType::LESS, "<"},
            {tokenType::LESSEQUAL, "<="},
            {tokenType::GREATER, ">"},
            {tokenType::GREATEREQUAL, ">="},
            {tokenType::AND, "&&"},
            {tokenType::OR, "||"},
            {tokenType::ref, "&"},
            {tokenType::scope, "::"},
            {tokenType::semicolon, ";"},
            {tokenType::comma, ","},
            {tokenType::leftparen, "("},
            {tokenType::rightparen, ")"},
            {tokenType::leftbrace, "{"},
            {tokenType::rightbrace, "}"}
        }
    );

    
    expectTokens(scanner, "string literals",
        R"TK("hello" "two words" "quote: \"ok\"" "backslash: \\")TK",
        {
            {tokenType::STRING_LITERAL, R"TK("hello")TK"},
            {tokenType::STRING_LITERAL, R"TK("two words")TK"},
            {tokenType::STRING_LITERAL, R"TK("quote: \"ok\"")TK"},
            {tokenType::STRING_LITERAL, R"TK("backslash: \\")TK"}
        }
    );

    
    expectTokens(scanner, "character literals",
        R"TK('a' 'Z' '7' '\n' '\'' '\\')TK",
        {
            {tokenType::CHAR_LITERAL, "'a'"},
            {tokenType::CHAR_LITERAL, "'Z'"},
            {tokenType::CHAR_LITERAL, "'7'"},
            {tokenType::CHAR_LITERAL, R"TK('\n')TK"},
            {tokenType::CHAR_LITERAL, R"TK('\'')TK"},
            {tokenType::CHAR_LITERAL, R"TK('\\')TK"}
        }
    );

    
    expectTokens(scanner, "whitespace and line comments",
        "int count = 10; // ignored: if, 999\n\tfloat ratio = 2.5;\n",
        {
            {tokenType::INTEGERTYPE, "int"},
            {tokenType::IDENTIFIER, "count"},
            {tokenType::ASSIGN, "="},
            {tokenType::INTEGER_LITERAL, "10"},
            {tokenType::semicolon, ";"},
            {tokenType::FLOATTYPE, "float"},
            {tokenType::IDENTIFIER, "ratio"},
            {tokenType::ASSIGN, "="},
            {tokenType::FLOAT_LITERAL, "2.5"},
            {tokenType::semicolon, ";"}
        }
    );

    
    expectTokens(scanner, "invalid characters",
        "@ : | .",
        {
            {tokenType::invalid, "@"},
            {tokenType::invalid, ":"},
            {tokenType::invalid, "|"},
            {tokenType::invalid, "."}
        }
    );

    
    expectTokens(scanner, "malformed character literals",
        R"TK('' 'ab' 'x)TK",
        {
            {tokenType::invalid, "''"},
            {tokenType::invalid, "'ab'"},
            {tokenType::invalid, "'x"}
        }
    );

    expectTokens(scanner, "unterminated string",
        R"TK("unfinished)TK",
        {
            {tokenType::invalid, R"TK("unfinished)TK"}
        }
    );

    
    expectTokens(scanner, "empty input", "", {});
    expectTokens(scanner, "comment-only input", "  // nothing here\n\t", {});

    
    expectTokens(scanner, "lexer reuse: keyword", "int",
        {{tokenType::INTEGERTYPE, "int"}}
    );
    expectTokens(scanner, "lexer reuse: identifier", "another_name",
        {{tokenType::IDENTIFIER, "another_name"}}
    );

    std::cout << "\nResults: " << passed << " passed, "
              << failed << " failed.\n";

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
