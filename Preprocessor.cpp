#include "Preprocessor.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

istream& separaTokenStream(istringstream& stream, string& str, char delim) {
    str.erase();
    char ch;
    while (stream.get(ch)) {
        if (ch == delim) {
            break;
        }
        str += ch;
    }
    return stream;
}

vector<string> Preprocessor::split(const string& s) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);

    cout << "Separando tokens na linha: " << s << endl;

    // Manipula a referência do objeto token e tokenStream. Por isso, não são necessárias variáveis temporárias para iterar e armazenar
    while (separaTokenStream(tokenStream, token, ' ')) {
        if (!token.empty()) {
            if (token.back() == ',') {
                token.pop_back();
            }
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
    }

    cout << "Tokens: ";
    for (const auto& t : tokens) {
        cout << "[" << t << "] ";
    }
    cout << endl;

    return tokens;
}

void Preprocessor::process(const string& inputFilename, const string& outputFilename) {
    ifstream inputFile(inputFilename);
    ofstream outputFile(outputFilename);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        throw runtime_error("Nao foi possivel abrir os arquivos de pre-processamento.");
    }

    string line;
    bool isMacro = false; // Flag pra inicio de macro
    MNTItem currentMacro;

    while (getline(inputFile, line)) {
        string upperLine = line;

        // Converte linha pra maiúsculas
        for (size_t i = 0; i < upperLine.size(); ++i) {
            upperLine[i] = toupper(static_cast<unsigned char>(upperLine[i]));
        }

        vector<string> tokens = split(upperLine);
        // Se a linha estiver vazia, apenas copia (se não estiver dentro de uma macro)
        if (tokens.empty()) {
            if (!isMacro) outputFile << line << endl;
            continue;
        }

        // Procura início de uma definição de macro.
        if (upperLine.find("MACRO")!= string::npos) {
            isMacro = true;
            
            // Extrai o nome da macro e seus parâmetros.
            string label_part = tokens;
            currentMacro.name = label_part.substr(0, label_part.find(':'));
            currentMacro.params.clear();
            for (size_t i = 2; i < tokens.size(); ++i) {
                currentMacro.params.push_back(tokens[i]);
            }
            currentMacro.mdt_start_index = mdt.size();
            continue; // Pula para a próxima linha sem escrever nada no arquivo.pre.
        }

        // Detecta o fim de uma definição de macro.
        if (upperLine.find("ENDMACRO")!= string::npos) {
            isMacro = false;
            mnt[currentMacro.name] = currentMacro; // Salva a macro na MNT.
            mdt.push_back("ENDMACRO"); // Adiciona um marcador de fim na MDT.
            continue; // Pula para a próxima linha.
        }

        // Se estiver no estado de definição, armazena a linha na MDT.
        if (isMacro) {
            string processed_line = line;
            // Substitui os nomes dos parâmetros por marcadores posicionais (ex: #1, #2).
            // Isso torna a expansão mais genérica. [1]
            for (size_t i = 0; i < currentMacro.params.size(); ++i) {
                string placeholder = "#" + to_string(i + 1);
                string param = currentMacro.params[i];
                size_t pos = processed_line.find(param);
                while(pos!= string::npos) {
                    processed_line.replace(pos, param.length(), placeholder);
                    pos = processed_line.find(param, pos + placeholder.length());
                }
            }
            mdt.push_back(processed_line);
        } else {
            // Se não estiver definindo, verifica se é uma chamada de macro.
            string potential_macro_name = tokens;
            if (potential_macro_name.back() == ':') {
                 potential_macro_name.pop_back();
            }
            
            // Se o primeiro token da linha é um nome de macro conhecido...
            if (mnt.count(potential_macro_name)) {
                MNTItem& macro = mnt[potential_macro_name];
                vector<string> args;
                // Coleta os argumentos da chamada.
                for (size_t i = (tokens.back() == ':'? 2 : 1); i < tokens.size(); ++i) {
                     args.push_back(tokens[i]);
                }

                // Lógica de expansão, incluindo tratamento de chamadas aninhadas usando uma pilha.
                vector<pair<int, vector<string>>> expansion_stack;
                expansion_stack.push_back({macro.mdt_start_index, args});

                while(!expansion_stack.empty()){
                    int mdt_idx = expansion_stack.back().first;
                    vector<string> current_args = expansion_stack.back().second;
                    expansion_stack.pop_back();

                    while (mdt[mdt_idx]!= "ENDMACRO") {
                        string macro_line = mdt[mdt_idx];
                        vector<string> macro_tokens = split(macro_line);
                        
                        // Verifica se uma linha dentro da macro é uma chamada para outra macro (aninhamento).
                        if (!macro_tokens.empty() && mnt.count(macro_tokens)) {
                            // Empilha o estado atual e inicia uma nova expansão.
                            expansion_stack.push_back({mdt_idx + 1, current_args});
                            
                            MNTItem& nested_macro = mnt[macro_tokens];
                            vector<string> nested_args;
                            for(size_t i = 1; i < macro_tokens.size(); ++i) {
                                string arg_token = macro_tokens[i];
                                if(arg_token == '#') { // Se o argumento é um parâmetro da macro externa.
                                    int param_idx = stoi(arg_token.substr(1)) - 1;
                                    if(param_idx < current_args.size()) {
                                        nested_args.push_back(current_args[param_idx]);
                                    }
                                } else {
                                    nested_args.push_back(arg_token);
                                }
                            }
                            expansion_stack.push_back({nested_macro.mdt_start_index, nested_args});
                            goto next_in_stack; // Pula para a próxima iteração do loop da pilha.
                        }

                        // Substitui os marcadores de parâmetro (#1, #2) pelos argumentos reais.
                        for (size_t i = 0; i < current_args.size(); ++i) {
                            string placeholder = "#" + to_string(i + 1);
                            size_t pos = macro_line.find(placeholder);
                            while (pos!= string::npos) {
                                macro_line.replace(pos, placeholder.length(), current_args[i]);
                                pos = macro_line.find(placeholder, pos + current_args[i].length());
                            }
                        }
                        outputFile << macro_line << endl;
                        mdt_idx++;
                    }
                    next_in_stack:;
                }
            } else {
                // Se não for uma definição nem uma chamada de macro, é código normal. Copia para a saída.
                outputFile << line << endl;
            }
        }
    }
}