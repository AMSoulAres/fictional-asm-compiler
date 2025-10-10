#include "Preprocessor.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

vector<string> Preprocessor::split(const string &s)
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);

    // Divide por espaços e vírgulas, removendo vírgulas do final dos tokens.
    while (tokenStream >> token)
    {
        if (!token.empty() && token.back() == ',')
        {
            token.pop_back();
        }
        if (!token.empty())
            tokens.push_back(token);
    }

    return tokens;
}

// Método privado para expandir uma macro, com suporte a chamadas aninhadas (recursão).
void Preprocessor:: expand_macro(const string& name, const vector<string>& args, ofstream& output_file) {
    // Busca a macro na Tabela de Nomes de Macro (MNT).
    const MNTItem& macroInfo = mnt.at(name);

    // Itera sobre o corpo da macro na Tabela de Definição de Macro (MDT).
    for (size_t i = macroInfo.mdtStartIndex; i < mdt.size(); i++) {
        string macroLine = mdt[i];
        cout << "Processando linha da macro: " << macroLine << endl;
        
        // Para a expansão ao encontrar o "ENDMACRO" da definição atual.
        if (mdt[i] == "ENDMACRO") break;

        // Substitui os marcadores de parâmetro (#1, #2) pelos argumentos reais da chamada.
        for (size_t j = 0; j < args.size(); j++) {
            string placeholder = "#" + to_string(j + 1);
            size_t pos = macroLine.find(placeholder);
            while (pos != string::npos) {
                macroLine.replace(pos, placeholder.length(), args[j]);
                cout << "Substituindo " << placeholder << " por " << args[j] << endl;
                pos = macroLine.find(placeholder, pos + args[j].length());
            }
        }
        
        // Após a substituição, verifica se a linha resultante é uma chamada de macro aninhada.
        vector<string> macroTokens = split(macroLine);
        if (!macroTokens.empty() && mnt.count(macroTokens[0])) {
            // É uma chamada aninhada. Coleta os argumentos e chama a si mesma recursivamente.
            cout << "Encontrada macro aninhada: " << macroTokens[0] << endl;
            vector<string> nested_args;
            for (size_t k = 1; k < macroTokens.size(); k++) {
                nested_args.push_back(macroTokens[k]);
                cout << "Argumento aninhado: " << macroTokens[k] << endl;
            }
            expand_macro(macroTokens[0], nested_args, output_file);
        } else {
            // Se não for uma chamada aninhada, escreve a linha expandida no arquivo de saída.
            output_file << macroLine << endl;
        }
    }
}

void Preprocessor::process(const string &inputFilename, const string &outputFilename)
{
    ifstream inputFile(inputFilename);
    ofstream outputFile(outputFilename);

    if (!inputFile.is_open() || !outputFile.is_open())
    {
        throw runtime_error("Nao foi possivel abrir os arquivos de pre-processamento.");
    }

    string line;
    bool isMacro = false; // Flag pra inicio de macro
    MNTItem currentMacro;

    while (getline(inputFile, line))
    {
        string upperLine = line;

        // Converte linha pra maiúsculas
        for (size_t i = 0; i < upperLine.size(); i++)
        {
            upperLine[i] = toupper(static_cast<unsigned char>(upperLine[i]));
        }

        vector<string> tokens = split(upperLine);
        // Se a linha estiver vazia, apenas copia (se não estiver dentro de uma macro)
        if (tokens.empty())
        {
            if (!isMacro)
                outputFile << line << endl;
            continue;
        }

        bool line_handled = false;
        for (const auto &token : tokens)
        {
            if (token == "MACRO")
            {
                isMacro = true;
                // Extrai o nome da macro e seus parâmetros. (Máximo de 2)
                string label_part = tokens[0];
                currentMacro.name = label_part.substr(0, label_part.find(':'));
                currentMacro.params.clear();
                for (size_t i = 2; i < tokens.size(); i++)
                {
                    currentMacro.params.push_back(tokens[i]);
                }
                currentMacro.mdtStartIndex = mdt.size();
                line_handled = true;
                break; 
            }

            // Detecta o fim de uma definição de macro.
            if (token == "ENDMACRO")
            {
                isMacro = false;
                mnt[currentMacro.name] = currentMacro; // Salva a macro na MNT.
                mdt.push_back("ENDMACRO");             // Adiciona um marcador de fim na MDT.
                line_handled = true;
                break;
            }
        }

        if (line_handled) {
            continue;
        }

        // Se estiver no estado de definição, armazena a linha na MDT.
        if (isMacro) {
            // Substitui os nomes dos parâmetros por marcadores posicionais (ex: #1, #2).
            for (size_t i = 0; i < currentMacro.params.size(); i++) {
                string placeholder = "#" + to_string(i + 1);
                string param = currentMacro.params[i];
                size_t pos = line.find(param);
                while(pos!= string::npos) {
                    line.replace(pos, param.length(), placeholder);
                    pos = line.find(param, pos + placeholder.length());
                }
            }
            mdt.push_back(line);
        } else {
            // Se não estiver definindo, verifica se é uma chamada de macro.
            string potentialMacro = tokens[0];
            if (potentialMacro.back() == ':') { 
                 potentialMacro.pop_back();
            }

            // Se o primeiro token da linha é um nome de macro conhecido
            // Usando como unordered_map para busca - retorna 0 ou 1
            if (mnt.count(potentialMacro)) {
                vector<string> args;
                
                // Para definição SWAP:           (TODO: Vai ser usada?)
                //                MACRO &A, &B, &T
                size_t start = 1;
                if (!tokens.empty() && tokens[0].back() == ':') {
                    // Para definição SWAP: MACRO &A, &B, &T
                    start = 2;
                }
                for (size_t i = start; i < tokens.size(); i++) {
                    args.push_back(tokens[i]);
                }

                cout << "Expansao da macro: " << potentialMacro << " com " << args.size() << " argumentos." << endl;
                expand_macro(potentialMacro, args, outputFile);

               
            } else {
                // Sem macros
                outputFile << line << endl;
            }
        }
    }
}