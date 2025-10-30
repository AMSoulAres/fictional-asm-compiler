#include "LexicalAnalyzer.hpp"
#include <cctype>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;

vector<Token> LexicalAnalyzer::tokenize(const string &source, int line)
{
    vector<Token> tokens;
    // Replace commas with spaces so stringstream splits on commas as separators
    string sanitized = source;
    replace(sanitized.begin(), sanitized.end(), ',', ' ');
    // Remove spaces around '+' so expressions like "N2 + 3" become "N2+3"
    string compact;
    for (size_t i = 0; i < sanitized.size(); ++i)
    {
        char c = sanitized[i];
        if (isspace(static_cast<unsigned char>(c)))
        {
            bool nextIsPlus = (i + 1 < sanitized.size() && sanitized[i + 1] == '+');
            bool prevIsPlus = (i > 0 && sanitized[i - 1] == '+');
            if (nextIsPlus || prevIsPlus)
                continue; // skip spaces adjacent to plus
        }
        compact.push_back(c);
    }
    stringstream stream(compact);
    string word;

    while (stream >> word)
    {
        Token token;
        token.lineNumber = line;

        if (word == ";") {
            break; // Ignore the rest of the line as it's a comment
        }

        if (!word.empty())
        {
            if (word.back() == ':')
            {
                word.pop_back();
                if (!isValidLabel(word))
                {
                    throw LexicalException("Rótulo inválido: " + word, line);
                }
                token.type = TokenType::LABEL;
                token.value = word;
            }
            else if (word == "SPACE" || word == "CONST")
            {
                // Não trata macros pq são excluídas no pré-processamento
                token.type = TokenType::DIRECTIVE;
                token.value = word;
            }
            else if (
                // Acha o token e retorna o indice ou retorna o último elemento
                find(optab.begin(), optab.end(), word) != optab.end())
            {
                token.type = TokenType::INSTRUCTION;
                token.value = word;
            }
            else if (all_of(word.begin(), word.end(), ::isdigit))
            {
                token.type = TokenType::NUMBER;
                token.value = word;
            }
            else
            {
                // Token desconhecido, assume como PARAMETER (pode ser um rótulo ou variável)
                token.type = TokenType::PARAMETER;
                token.value = word;

                // Valida parameteros nos formatos: LABEL, LABEL+NUMBER, NUMBER+NUMBER, etc.
                size_t plusPos = word.find('+');
                if (plusPos != string::npos)
                {
                    string left = word.substr(0, plusPos);
                    string right = word.substr(plusPos + 1);
                    bool leftValid = !left.empty() && (isValidLabel(left) || all_of(left.begin(), left.end(), ::isdigit));
                    bool rightValid = !right.empty() && all_of(right.begin(), right.end(), ::isdigit);
                    if (!leftValid || !rightValid)
                    {
                        throw LexicalException("Parametero ou rótulo inválido: " + word, line);
                    }
                }
                else
                {
                    if (!isValidLabel(word))
                    {
                        throw LexicalException("Parametero ou rótulo inválido: " + word, line);
                    }
                }
            }

            tokens.push_back(token); 
        }

    }
    return tokens;
}

bool LexicalAnalyzer::isValidLabel(const string &label)
{
    if (label.empty())
        return false;

    if (!isalpha(label[0]) && label[0] != '_')
        return false;

    for (size_t i = 1; i < label.size(); i++)
    {
        if (!isalnum(label[i]) && label[i] != '_')
        {
            return false;
        }
    }
    return true;
}
