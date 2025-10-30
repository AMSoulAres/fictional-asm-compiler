#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

// #include "Lexer.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include "LexicalAnalyzer.hpp"

using namespace std;

// Opcodes, tamanhos e número de parameteros.
struct OpInfo {
    int opcode;
    int size;           // Tamanho em words de memória.
    int numParameters;
};

// Item da Tabela de Símbolos.
struct SymbolItem {
    int address;
    bool isDefined = false; 
    int pendingListHead = -1; // Lista de endereços no código objeto que precisam ser corrigidos.
};

class Assembler {
public:
    // Método principal que orquestra a montagem.
    void assemble(const string& input_filename
        //  const string& o1_filename, const string& o2_filename
        );

private:
    // Métodos privados que implementam as fases da montagem.
    void initialize_optab();
    void first_pass(const string& input_filename);
    void generate_o1_file(const string& filename);
    void resolvePendingReferences();
    // void generate_o2_file(const string& filename);
    
    LexicalAnalyzer lexicalAnalyzer;

    // Tabela de Operações
    unordered_map<string, OpInfo> optab;

    // Tabela de Símbolos
    unordered_map<string, SymbolItem> symtab;

    vector<int> codigoObjeto;

    vector<pair<int, string>> errors;
};

#endif // ASSEMBLER_HPP