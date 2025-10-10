#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

// #include "Lexer.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <list>

using namespace std;

// Opcodes, tamanhos e número de operandos.
struct OpInfo {
    string name;
    int opcode;
    int size;           // Tamanho em words de memória.
    int num_operands;
};

// Item da Tabela de Símbolos.
struct SymbolItem {
    int address;
    bool is_defined = false; 
    list<int> pending_list; // Lista de endereços no código objeto que precisam ser corrigidos.
};

class Assembler {
public:
    // Método principal que orquestra a montagem.
    void assemble(const string& input_filename, const string& o1_filename, const string& o2_filename);

private:
    // Métodos privados que implementam as fases da montagem.
    void initialize_optab();
    void first_pass(const string& input_filename);
    void generate_o1_file(const string& filename);
    void backpatch();
    void generate_o2_file(const string& filename);
    
    // Analisador léxico para tokenizar o código.
    // Lexer lexer;
    // Tabela de Operações (OPTAB).
    unordered_map<string, OpInfo> optab;
    // Tabela de Símbolos (SYMTAB).
    unordered_map<string, SymbolItem> symtab;
    // Vetor para armazenar o código objeto gerado.
    vector<int> object_code;
    // Vetor para armazenar erros encontrados durante a montagem.
    vector<pair<int, string>> errors;
};

#endif // ASSEMBLER_HPP