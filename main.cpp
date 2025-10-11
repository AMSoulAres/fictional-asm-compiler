#include "Preprocessor.hpp"
// #include "Assembler.hpp"
#include <iostream>
#include <string>
#include "Assembler.hpp"

using namespace std;

string change_extension(const string& filename, const string& new_ext) {
    size_t last_dot = filename.find_last_of(".");
    if (last_dot == string::npos) {
        return filename + new_ext;
    }
    return filename.substr(0, last_dot) + new_ext;
}

int main(int argc, char* argv[]) {
    string input_filename;

    // Valida os argumentos da linha de comando.
    if (argc == 2) {
        input_filename = argv[1];
    } else {
        cout << "Nenhum arquivo informado. Usando example.asm para debug." << endl;
        input_filename = "example.asm";
    }
    string pre_filename = change_extension(input_filename, ".pre");
    string o1_filename = change_extension(input_filename, ".o1");
    string o2_filename = change_extension(input_filename, ".o2");

    try {
        // Pré-processamento
        cout << "Iniciando Pre-processamento..." << endl;
        Preprocessor preprocessor;
        preprocessor.process(input_filename, pre_filename);
        cout << "Pre-processamento concluido. Saida em: " << pre_filename << endl;

        // Executa a Passagem 1: Montagem.
        cout << "Iniciando Passagem 1: Montagem..." << endl;
        Assembler assembler;
        assembler.assemble(pre_filename);
        // cout << "Montagem concluida. Saidas em: " << o1_filename << " e " << o2_filename << endl;

    } catch (const exception& e) {
        cerr << "Erro fatal durante a compilacao: " << e.what() << endl;
        return 1;
    }

    return 0;
}