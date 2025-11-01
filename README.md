
# Montador / Pré-processador (Projeto de Software Básico)

Este repositório contém uma implementação em C++ de um pré-processador e um
montador (assembler) para uma máquina hipotética usada nas atividades de
Software Básico. O projeto gera dois artefatos de objeto: `.o1` (com pendências
preservadas) e `.o2` (com endereços resolvidos).

O projeto apenas a std lib.

## Escopo

- Pré-processador com suporte a macros (definição/expansão), incluindo macros
	aninhadas.
- Analisador léxico (tokenizador): trata vírgulas, comentarios, valida
	nomes de rótulos (alfanuméricos e underscore, iniciando por letra ou `_`), e
	aceita expressões do tipo `LABEL+offset`.
- Montador single-pass com lista de pendências embutida no próprio código-objeto
	(`codigoObjeto`) e offsets das pendências armazenados externamente. Ao
	definir um rótulo, o montador resolve imediatamente as pendências, mas cria
	um snapshot não-resolvido (`.o1`) antes das correções para que o arquivo
	`.o1` retenha os placeholders e a lista encadeada de pendências.

## Arquitetura / Como funciona

- `Preprocessor` (arquivo `Preprocessor.cpp/.hpp`)
	- Lê o `.asm` de entrada, processa a definição de macros (MNT/MDT) e
		expande chamadas de macros, gerando o arquivo `.pre` (pré-processado).
- `LexicalAnalyzer` (arquivo `LexicalAnalyzer.cpp/.hpp`)
	- Tokeniza linhas do `.pre`, substitui vírgulas por separadores, remove
		espaços ao redor de `+` para unificar `LABEL + 3` e `LABEL+3`, e valida
		tokens. Lança `LexicalException` com número da linha em casos de erro.
- `Assembler` (arquivo `Assembler.cpp/.hpp`)
	- Implementa a `pass` (passagem 1) que consome o `.pre`, constrói a tabela
		de símbolos (`symtab`), emite palavras no `codigoObjeto` e gerencia a
		lista de pendências. Possui:
		- `codigoObjetoO1`: snapshot do objeto antes de backpatch (escrito em `.o1`).
		- `codigoObjeto`: objeto que será corrigido quando rótulos forem definidos
			e escrito em `.o2`.
		- `pendingOffsets`: map de índice em `codigoObjeto` -> offset associado à
			pendência.

## Arquivos principais

- `main.cpp` — orquestra: chama o pré-processador e o montador.
- `Preprocessor.*` — expande macros e escreve arquivo `.pre`.
- `LexicalAnalyzer.*`, `Token.hpp` — tokenização e validação léxica.
- `Assembler.*` — montagem do código; geração de `*.o1` e `*.o2`.
- `example.asm` — conjunto de testes válidos (casos de uso do montador).
- `exampleErrors.asm` — testes que devem produzir erros léxicos/semânticos.

## Como compilar

No diretório do projeto, execute (Linux / zsh):

```bash
g++ -std=c++17 -Wall -Wextra -I. main.cpp Preprocessor.cpp LexicalAnalyzer.cpp Assembler.cpp -o compiler
```

Isso produzirá o executável `compiler`.

## Como rodar

1. Para pré-processar e montar o arquivo de exemplo `example.asm` (ou outro
	 arquivo `.asm` que você passar como argumento):

```bash
./compiler example.asm
```

2. Saídas geradas (mesmo prefixo do arquivo de entrada):
	 - `example.pre` — resultado do pré-processamento (expansão de macros).
	 - `example.o1`  — código-objeto com pendências preservadas (placeholders
		 e lista encadeada dentro do objeto).
	 - `example.o2`  — código-objeto com pendências resolvidas (endereços
		 definitivos).

## Observações e detalhes de uso

- O montador é case-insensitive (todas as linhas são convertidas para maiúsculas
	antes da tokenização).
- Comentários iniciam com `;` e o restante da linha é ignorado.
- Diretivas suportadas: `SPACE` e `CONST` (ver `example.asm` e
	`exampleErrors.asm` para exemplos e casos de erro).
- Operandos podem ser rótulos, literais numéricos ou expressões do tipo
	`LABEL+offset` (onde `offset` é um número decimal não-negativo).
- A linguagem utilizada (assemply hipotético) tem a seguinte composição:
```
Opcode   | Opcode   | Tamanho     |Ação
Simbólico| Numérico | (words)     |

ADD      | 01       | 2           | ACC <- ACC + mem(OP)
SUB      | 02       | 2           | ACC <- ACC - mem(OP)
MUL      | 03       | 2           | ACC <- ACC × mem(OP)
DIV      | 04       | 2           | ACC <- ACC ÷ mem(OP)
JMP      | 05       | 2           | PC <- OP
JMPN     | 06       | 2           | Se ACC<0 então PC <- OP
JMPP     | 07       | 2           | Se ACC>0 então PC <- OP
JMPZ     | 08       | 2           | Se ACC=0 então PC <- OP
COPY     | 09       | 3           | mem(OP2) <- mem(OP1)
LOAD     | 10       | 2           | ACC <- mem(OP)
STORE    | 11       | 2           | mem(OP) <- ACC
INPUT    | 12       | 2           | mem(OP) <- entrada
OUTPUT   | 13       | 2           | saída ß mem(OP)
STOP     | 14       | 1           | Suspende a execução


Exemplo de código:
INPUT N1
INPUT N2
LOAD N1
ADD N2
STORE N3
OUTPUT N3
STOP
N1: SPACE
N2: SPACE
N3: SPACE


Arquivo gerado:
End 0 12 13
End 2 12 14
End 4 10 13
End 6 01 14
End 8 11 15
End 10 13 15
End 12 14
End 13 xx
End 14 xx
End 15 xx
```

## Depuração e mensagens

- O montador imprime mensagens de pré-processamento (expansão de macros),
	mensagens de montagem e imprime as pendências detectadas (por símbolo e
	lista global) como saída de debug.

