#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include "Token.hpp"

using namespace std;

class LexicalAnalyzer {
public:
    vector<Token> tokenize(const string& source, int line);

private:
    // string cleanLine(const string& line);
    bool isValidLabel(const string& label);
    vector<string> optab = {
        "ADD", "SUB", "MULT", "DIV", "JMP", "JMPN", "JMPP", "JMPZ",
        "COPY", "LOAD", "STORE", "INPUT", "OUTPUT", "STOP"
    };
    string source;
    size_t currentPos;
};

class LexicalException : public runtime_error {
public:
    LexicalException(const string& message, int lineNumber) : runtime_error(message), lineNumber(lineNumber) {}
    int getLineNumber() const { return lineNumber; }

private:
    int lineNumber;
};

#endif // LEXICAL_ANALYZER_HPP