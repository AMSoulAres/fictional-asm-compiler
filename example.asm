; example.asm - comprehensive test cases for the assembler
; Sections: valid code, macros, forward/back references, directives, errors

; ---------- Inputs and simple ops ----------
INPUT N1
INPUT N2
INPUT A
LOAD N1
ADD N2 + 3       ; test LABEL + offset (with spaces)
ADD N1+2         ; test LABEL+offset (no spaces)
STORE N3
OUTPUT N3
STOP

; ---------- Data directives ----------
N1: SPACE
N2: SPACE 2
N3: SPACE
A:  SPACE 10

; ---------- Macros (including nested) ----------
SWAP: MACRO &A, &B
COPY &B, &A
COPY &A, &B
ENDMACRO

INC_A: MACRO
	LOAD A
	ADD 1
ENDMACRO

SWAP N1, N2
INC_A

; ---------- Valid forward reference (should create pendência then resolve) ----------
JMP future_label

; label alone on a line and instruction on next (pendingDefinition test)
alone_label:
	ADD N1

; define future label (resolves previous JMP)
future_label: STOP

; Test immediate numeric operand (allowed by assembler)
ADD 1


; End of testszzz



