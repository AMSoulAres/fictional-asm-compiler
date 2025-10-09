#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <string>
#include <vector>
#include <unordered_map>

// Tabela de Nomes de Macro (MNT).
// Contém o nome, a lista de parâmetros e onde é definida na MDT.
struct MNTItem {
    std::string name;
    std::vector<std::string> params;
    int mdt_start_index;
};

// Lê o arquivo.asm, expande todas as macros e gera o arquivo.pre.
class Preprocessor {
public:
    void process(const std::string& input_filename, const std::string& output_filename);

private:
    // Tabela de Nomes de Macro (MNT): associa nomes das macros às suas informações.
    std::unordered_map<std::string, MNTItem> mnt;
    // Tabela de Definição de Macro (MDT): armazena o corpo de todas as macros. (Referenciada pelo indice mdt_start_index na MNT)
    std::vector<std::string> mdt;

    // Função para dividir uma linha em tokens.
    std::vector<std::string> split(const std::string& s);
};

#endif // PREPROCESSOR_HPP