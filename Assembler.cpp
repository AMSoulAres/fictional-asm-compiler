#include "Assembler.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

void Assembler::assemble(const string &input_filename, const string &o1_filename, const string &o2_filename)
{
    initialize_optab();
    pass(input_filename);

    if (!errors.empty())
    {
        cout << "Erros encontrados durante a montagem:\n";
        for (const auto &err : errors)
        {
            cout << "Linha " << err.first << ": " << err.second << endl;
        }
    }
    generate_o1_file(o1_filename);
    generate_o2_file(o2_filename);
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

void Assembler::pass(const string &input_filename)
{
    ifstream input_file(input_filename);
    if (!input_file.is_open())
    {
        throw runtime_error("Erro ao abrir o arquivo de entrada: " + input_filename);
    }

    string line;
    int locCounter = 0;
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
                // Se a linha não tem instrução (só rótulo),
                // seta como pendente e espera a definição real
                // de uma instrução na mesma linha ou em uma linha seguinte
                // Caso: ROTULO:
                //         ADD N1
                if (tokenIndex >= tokens.size())
                {
                    pendingDefinition = currentLabel;
                    continue;
                }

                // Se não, define o rótulo agora (instrução segue na mesma linha)
                // Caso: ROTULO: ADD N1
                if (symtab.count(currentLabel) && symtab[currentLabel].isDefined)
                {
                    throw runtime_error("Rotulo '" + currentLabel + "' declarado duas vezes.");
                }

                // Adiciona ou atualiza o rótulo na Tabela de Símbolos (SYMTAB)
                symtab[currentLabel].address = locCounter;
                symtab[currentLabel].isDefined = true;

                // Resolve as pendências do rótulo na sua declaração
                if (symtab[currentLabel].pendingListHead != -1) {
                    int cur = symtab[currentLabel].pendingListHead;
                    while (cur != -1) {
                        if (cur < 0 || static_cast<size_t>(cur) >= codigoObjeto.size()) {
                            throw runtime_error("Lista de pendencias corrompida ao resolver rotulo: " + currentLabel);
                        }
                        int nextLoc = codigoObjeto[cur];
                        int offset = 0;
                        auto itOff = pendingOffsets.find(cur);
                        if (itOff != pendingOffsets.end()) {
                            offset = itOff->second;
                            pendingOffsets.erase(itOff);
                        }
                        // escreve o endereço final (endereço do símbolo + offset)
                        codigoObjeto[cur] = symtab[currentLabel].address + offset;
                        cur = nextLoc;
                    }
                    // limpa a cabeça da lista de pendências
                    symtab[currentLabel].pendingListHead = -1;
                }
                // Se existia uma definição pendente usada aqui, dá um clear para
                // não considerar o mesmo rótulo de novo nas próximas linhas
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
                    i += 1;
                    operands.push_back(op);
                }

                if (operands.size() != static_cast<size_t>(opInfo.numParameters)) {
                    throw runtime_error("Instrução '" + mainToken.value + "' com número de parâmetros errado.");
                }

                // grava opcode em ambas as representações (O1 mantém pendências)
                codigoObjeto.push_back(opInfo.opcode);
                codigoObjetoO1.push_back(opInfo.opcode);

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

                    // Suporte pra imediatos (apesar de não serem permitidos na especificação)
                    if (plusPos == std::string::npos && !base.empty() && all_of(base.begin(), base.end(), ::isdigit)) {
                        int imm = stoi(base);
                        codigoObjeto.push_back(imm);
                        codigoObjetoO1.push_back(imm);
                        continue;
                    }

                    if (symtab.count(base) && symtab[base].isDefined) {
                        int resolvedVal = symtab[base].address + offset;
                        codigoObjeto.push_back(resolvedVal);
                        codigoObjetoO1.push_back(resolvedVal);
                    } else {
                        int previousHead = -1;
                        if (symtab.count(base)){
                            previousHead = symtab[base].pendingListHead;
                        } else {
                            // Inicializa entrada na SYMTAB se não existir
                            symtab[base];
                        }

                        // posição atual do código
                        int loc = static_cast<int>(codigoObjeto.size());

                        // placeholder no código objeto (guarda o head anterior como 'next')
                        codigoObjeto.push_back(previousHead);
                        // grava a mesma placeholder no O1 para n resolver pendencia
                        codigoObjetoO1.push_back(previousHead);

                        // armazena offset e link para a lista ligada em maps separados
                        pendingOffsets[loc] = offset;

                        symtab[base].pendingListHead = loc;
                    }
                }

                locCounter += opInfo.size;
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
                    int constVal = stoi(param);
                    codigoObjeto.push_back(constVal);
                    codigoObjetoO1.push_back(constVal);
                    locCounter += 1;
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
                        codigoObjetoO1.push_back(0);
                    }
                    locCounter += numSpaces;
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
            int errLine = le.getLineNumber() > 0 ? le.getLineNumber() : lineNumber;
            errors.push_back({errLine, string("Léxico: ") + le.what()});
        }
        catch (const runtime_error &e)
        {
            errors.push_back({lineNumber, e.what()});
        }
    }

    if (!pendingDefinition.empty())
    {
        errors.push_back({lineNumber, "Rótulo '" + pendingDefinition + "' declarado sem instrução."});
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
    for (size_t i = 0; i < codigoObjetoO1.size(); i++)
    {
        o1_file << codigoObjetoO1[i];
        if (i < codigoObjetoO1.size() - 1)
        {
            o1_file << " ";
        }
    }
    o1_file.close();
}

void Assembler::generate_o2_file(const string &o2_filename)
{
    ofstream o2_file(o2_filename);
    if (!o2_file.is_open())
    {
        throw runtime_error("Erro ao abrir o arquivo de saída O2: " + o2_filename);
    }

    printf("Gerando arquivo O2 (resolvido): %s\n", o2_filename.c_str());
    for (size_t i = 0; i < codigoObjeto.size(); i++)
    {
        o2_file << codigoObjeto[i];
        if (i < codigoObjeto.size() - 1)
        {
            o2_file << " ";
        }
    }
    o2_file.close();
}