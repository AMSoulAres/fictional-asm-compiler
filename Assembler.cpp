#include "Assembler.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

void Assembler::assemble(const string &input_filename
    //  const string &o1_filename, 
    //  const string &o2_filename
    )
{
    initialize_optab();
    first_pass(input_filename);

    // Print collected errors (lexical/syntactic/semantic) so they're visible to the user.
    if (!errors.empty())
    {
        cout << "Erros encontrados durante a montagem:\n";
        for (const auto &err : errors)
        {
            cout << "Linha " << err.first << ": " << err.second << endl;
        }
    }
    // generate_o1_file(o1_filename);
    // backpatch();
    // generate_o2_file(o2_filename);
}

void Assembler::initialize_optab()
{
    optab["ADD"] = {1, 2, 1};
    optab["SUB"] = {2, 2, 1};
    optab["MULT"] = {3, 2, 1};
    optab["DIV"] = {4, 2, 1};
    optab["JMP"] = {5, 2, 1};
    optab["JMPN"] = {6, 2, 1};
    optab["JMPP"] = {7, 2, 1};
    optab["JMPZ"] = {8, 2, 1};
    optab["COPY"] = {9, 3, 2};
    optab["LOAD"] = {10, 2, 1};
    optab["STORE"] = {11, 2, 1};
    optab["INPUT"] = {12, 2, 1};
    optab["OUTPUT"] = {13, 2, 1};
    optab["STOP"] = {14, 1, 0};
}

void Assembler::first_pass(const string &input_filename)
{
    ifstream input_file(input_filename);
    if (!input_file.is_open())
    {
        throw runtime_error("Erro ao abrir o arquivo de entrada: " + input_filename);
    }

    string line;
    // int locCounter = 0;
    int lineNumber = 0;
    string pendingDefinition = "";

    while (getline(input_file, line))
    {
        lineNumber++;
        for (char &c : line)
        {
            c = toupper(static_cast<unsigned char>(c));
        }

        try
        {
            vector<Token> tokens;
            // size_t pos = 0;
            if (line.empty() || all_of(line.begin(), line.end(), [](char c){ return isspace(static_cast<unsigned char>(c)); }) || line[0] == ';')
            {
                continue; // Pula linhas vazias ou comentários
            }
            tokens = lexicalAnalyzer.tokenize(line, lineNumber);
            if (tokens.empty())
            {
                continue;
            }

            // for (const Token &token : tokens)
            // {
            //     cout << "Token: " << token.value << " (Tipo: " << static_cast<int>(token.type) << ") na linha " << token.lineNumber << endl;
            // }
        }
        catch (const LexicalException &le)
        {
            // Use the line number reported by the lexical exception if available
            int errLine = le.getLineNumber() > 0 ? le.getLineNumber() : lineNumber;
            errors.push_back({errLine, string("Léxico: ") + le.what()});
        }
        catch (const runtime_error &e)
        {
            errors.push_back({lineNumber, e.what()});
        }
    }
}
