#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

// Tabela de Nomes de Macro (MNT).
// Contém o nome, a lista de parâmetros e onde é definida na MDT.
struct MNTItem {
    string name;
    vector<string> params;
    int mdtStartIndex;
};

// Lê o arquivo.asm, expande todas as macros e gera o arquivo.pre.
class Preprocessor {
public:
    void process(const string& input_filename, const string& output_filename);

private:
    // Tabela de Nomes de Macro (MNT): associa nomes das macros às suas informações.
    unordered_map<string, MNTItem> mnt;
    // Tabela de Definição de Macro (MDT): armazena o corpo de todas as macros. (Referenciada pelo indice mdtStartIndex na MNT)
    vector<string> mdt;

    // Função para dividir uma linha em tokens.
    vector<string> split(const string& s);
    void expand_macro(const string& name, const vector<string>& args, ofstream& output_file);


};

#endif // PREPROCESSOR_HPP