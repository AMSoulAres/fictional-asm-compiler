#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include "LexicalAnalyzer.hpp"

using namespace std;

struct OpInfo {
    int opcode;
    int size;           // Tamanho em words de memória.
    int numParameters;
};

// Item da Tabela de Símbolos.
struct SymbolItem {
    int address;
    bool isDefined = false; 
    int pendingListHead = -1;
};

class Assembler {
public:
    void assemble(const string& input_filename, const string& o1_filename, const string& o2_filename);

private:
    void initialize_optab();
    void pass(const string& input_filename);
    void generate_o1_file(const string& filename);
    void generate_o2_file(const string& filename);
    
    LexicalAnalyzer lexicalAnalyzer;

    // Tabela de Operações
    unordered_map<string, OpInfo> optab;

    // Tabela de Símbolos
    unordered_map<string, SymbolItem> symtab;

    vector<int> codigoObjeto;
    // (para gerar .o1)
    vector<int> codigoObjetoO1;

    // índice no codigoObjeto -> offset pendente
    std::unordered_map<int,int> pendingOffsets;

    vector<pair<int, string>> errors;
};

#endif // ASSEMBLER_HPP