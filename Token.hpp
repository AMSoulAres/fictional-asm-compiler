#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <vector>

using namespace std;

enum class TokenType {
    INSTRUCTION,
    LABEL,
    DIRECTIVE,
    OPERAND,
    NUMBER,
    COMMA,
    UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    int lineNumber;
};

vector<Token> tokenize(const string& line);

#endif