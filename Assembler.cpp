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
    int programCounter = 0;
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
            if (line.empty() || all_of(line.begin(), line.end(), [](char c)
                                       { return isspace(static_cast<unsigned char>(c)); }) ||
                line[0] == ';')
            {
                continue; // Pula linhas vazias ou comentários
            }
            tokens = lexicalAnalyzer.tokenize(line, lineNumber);
            if (tokens.empty())
            {
                continue;
            }
            size_t tokenIndex = 0;
            std::string currentLabel = pendingDefinition;

            if (tokens[0].type == TokenType::LABEL)
            {
                if (!currentLabel.empty())
                {
                    throw std::runtime_error("Dois rotulos na mesma linha.");
                }
                currentLabel = tokens[0].value;
                tokenIndex++;
            }

            if (!currentLabel.empty())
            {
                // If the current line contains only a label (no instruction/operands),
                // set it as pending and defer the actual definition until we have
                // an instruction on the same or a following line.
                if (tokenIndex >= tokens.size())
                {
                    pendingDefinition = currentLabel;
                    continue;
                }

                // Otherwise, define the label now (instruction follows on the same line).
                if (symtab.count(currentLabel) && symtab[currentLabel].isDefined)
                {
                    throw runtime_error("Rotulo '" + currentLabel + "' declarado duas vezes.");
                }

                // Adiciona ou atualiza o rótulo na Tabela de Símbolos (SYMTAB).
                symtab[currentLabel].address = programCounter;
                symtab[currentLabel].isDefined = true;

                // Se havia uma definição pendente usada aqui, limpamos para
                // não considerar o mesmo rótulo novamente nas próximas linhas.
                if (pendingDefinition == currentLabel)
                {
                    pendingDefinition.clear();
                }
            }

            Token mainToken = tokens[tokenIndex];

            if (mainToken.type == TokenType::INSTRUCTION)
            {
                const OpInfo &opInfo = optab.at(mainToken.value);

                // Parse dos operandos para: LABEL+3 or LABEL + 3
                std::vector<std::string> operands;
                size_t i = tokenIndex + 1;
                while (i < tokens.size()) {
                    std::string op = tokens[i].value;
                    if (i + 2 < tokens.size() && tokens[i+1].value == "+") {
                        op += "+" + tokens[i+2].value;
                        i += 3;
                    } else {
                        i += 1;
                    }
                    operands.push_back(op);
                }

                if (operands.size() != static_cast<size_t>(opInfo.numParameters)) {
                    throw runtime_error("Instrução '" + mainToken.value + "' com número de parâmetros errado.");
                }

                codigoObjeto.push_back(opInfo.opcode);

                for (size_t pi = 0; pi < operands.size(); ++pi) {
                    std::string param = operands[pi];

                    // Detecta expressão LABEL+offset
                    size_t plusPos = param.find('+');
                    std::string base = param;
                    int offset = 0;
                    if (plusPos != std::string::npos) {
                        base = param.substr(0, plusPos);
                        std::string offStr = param.substr(plusPos + 1);
                        if (offStr.empty() || !all_of(offStr.begin(), offStr.end(), ::isdigit)) {
                            throw runtime_error("Operando inválido: " + param);
                        }
                        offset = stoi(offStr);
                    }

                    // If immediate number (no base label)
                    if (plusPos == std::string::npos && !base.empty() && all_of(base.begin(), base.end(), ::isdigit)) {
                        codigoObjeto.push_back(stoi(base));
                        continue;
                    }

                    if (symtab.count(base) && symtab[base].isDefined) {
                        codigoObjeto.push_back(symtab[base].address + offset);
                    } else {
                        int previous_head = -1;
                        if (symtab.count(base)) previous_head = symtab[base].pendingListHead;
                        int loc = programCounter + 1 + static_cast<int>(pi);
                        // Encode: ((previous_head + 1) << 16) | (offset & 0xFFFF)
                        int encoded = ((previous_head + 1) << 16) | (offset & 0xFFFF);
                        codigoObjeto.push_back(encoded);
                        symtab[base].pendingListHead = loc;
                    }
                }

                programCounter += opInfo.size;
            }
            else if (mainToken.type == TokenType::DIRECTIVE)
            {
                if (mainToken.value == "CONST")
                {
                    if (tokens.size() <= tokenIndex + 1)
                    {
                        throw runtime_error("Diretiva CONST com número de parâmetros errado.");
                    }
                    string param = tokens[tokenIndex + 1].value;

                    if (!all_of(param.begin(), param.end(), ::isdigit))
                    {
                        throw runtime_error("Valor de CONST não é um número válido.");
                    }
                    codigoObjeto.push_back(stoi(param));
                    programCounter += 1;
                }
                else if (mainToken.value == "SPACE")
                {
                    int numSpaces = 1; // Default
                    if (tokens.size() - tokenIndex == 2)
                    {
                        string param = tokens[tokenIndex + 1].value;

                        if (!all_of(param.begin(), param.end(), ::isdigit))
                        {
                            throw runtime_error("Valor de SPACE não é um número válido.");
                        }
                        numSpaces = stoi(param);

                        if (numSpaces <= 0)
                        {
                            throw runtime_error("Valor de SPACE deve ser positivo.");
                        }
                    }
                    else if (tokens.size() - tokenIndex > 2)
                    {
                        throw runtime_error("Diretiva SPACE com numero de parametros errado.");
                    }
                    for (int i = 0; i < numSpaces; i++)
                    {
                        codigoObjeto.push_back(0); // Inicializa espaços com zero
                    }
                    programCounter += numSpaces;
                }
                else
                {
                    throw runtime_error("Diretiva desconhecida: " + mainToken.value);
                }
                pendingDefinition = ""; // Diretivas não podem deixar rótulo pendente
            }
            else
            {
                throw runtime_error("Instrução não reconhecida: " + mainToken.value);
            }
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

    for (const auto &pair : symtab)
    {
        if (!pair.second.isDefined)
        {
            errors.push_back({-1, "Erro Semântico: Rótulo '" + pair.first + "' não declarado."});
        }
    }
}

void Assembler::generate_o1_file(const string &o1_filename)
{
    ofstream o1_file(o1_filename);
    if (!o1_file.is_open())
    {
        throw runtime_error("Erro ao abrir o arquivo de saída O1: " + o1_filename);
    }

    printf("Gerando arquivo O1: %s\n", o1_filename.c_str());
    for (size_t i = 0; i < codigoObjeto.size(); i++)
    {
        o1_file << codigoObjeto[i];
        if (i < codigoObjeto.size() - 1)
        {
            o1_file << " ";
        }
    }
    o1_file.close();
}

void Assembler::resolvePendingReferences()
{
    for (auto &pair : symtab)
    {
        SymbolItem &item = pair.second;
        if (item.isDefined)
        {
            int currentLoc = item.pendingListHead;
            while (currentLoc != -1)
            {
                int encoded = codigoObjeto[currentLoc];
                int next_plus = (encoded >> 16);
                int nextLoc = next_plus - 1;
                int offset = encoded & 0xFFFF;
                codigoObjeto[currentLoc] = item.address + offset;
                currentLoc = nextLoc;
            }
        }
    }   
}