; exampleErrors.asm - test cases for lexical and semantic errors
; This file contains many cases to exercise lexer and single-pass assembler
; - lexical errors (invalid label names, invalid chars)
; - semantic errors (duplicate labels, two labels in same line, undefined labels,
;   instruction with wrong number of parameters, nonexistent instructions)
; - directives: SPACE and CONST (with and without arguments)
; - forward references to generate pendências (pending list)
; - mixed case to test case-insensitivity
; - whitespace variations: tabs, extra spaces, blank lines, newline after label

; ---------- Lexical errors ----------
1badlabel:    MOV A, B        ; invalid label (starts with digit) - lexical
BAD$CHAR:     ADD A, B        ; invalid character '$' in label - lexical

; Two labels on the same line (should be an error)
TWO_LABELS1: TWO_LABELS2: NOP ; syntactic/semantic error: two labels same line

; Label with only underscore is valid, but label that contains other special chars not
_valid_label: MOV A, B

; ---------- Semantic errors & pending references ----------
; Duplicate label (case-insensitive) declared later
dupLabel:     NOP
.ignore:      ; a comment-like label (if your assembler doesn't accept '.' as start, this triggers lexical)
DUPlabel:     ADD A, B       ; duplicate label (case-insensitive) - semantic

; Undefined label reference (never defined) - should remain in .o1 pendências
JMP neverDefined

; Forward reference that will be defined later (should create pendência then resolved by assembler if single-pass with linking step)
JMP futureLabel

; A label defined after use (valid forward reference)

; Example of label alone on a line followed by instruction on next line (should be accepted)
WAIT_LABEL:
    ADD N1

; Another forward reference with offset expression
LOAD VAL1+2

; Define FUTURE label so forward references can be resolved in a second pass or by resolver
futureLabel:   NOP

; Define VAL1 after use (tests pendências resolution)
VAL1:          CONST 7

; ---------- Directive misuse (semantic or lexical) ----------
; CONST expects a numeric argument; here it's invalid
BAD_CONST:     CONST ABC     ; semantic error: CONST requires numeric value

; SPACE with non-numeric argument
BAD_SPACE:     SPACE ZZZ     ; semantic error: SPACE expects a numeric argument

; Valid directives for contrast
SPACE_ONE:     SPACE 1
space_two:     space 2      ; lowercase directive to test case-insensitivity
CONST_OK:      CONST 10

; ---------- Instruction parameter count errors ----------
; Assuming MOV expects 2 parameters, ADD/SUB 2, NOP 0, JMP 1
MOV_WRONG:     MOV A         ; wrong number of parameters - semantic
ADD_TOO_MANY:  ADD A, B, C   ; wrong number of parameters - semantic
NOP_HAS_ARG:   NOP 1         ; NOP should not accept parameters - semantic

; ---------- Nonexistent instruction ----------
FOOBAR:        FOO X, Y      ; instruction doesn't exist - semantic

; ---------- Lexical tricky tokens ----------
; Comma-handling and fused tokens
MOV C,D,E      ; too many parameters and comma fused tokens (lexer should split commas)

; Invalid numeric literal
NUM_BAD:       CONST 12A    ; lexical/semantic – invalid number format

; Parameter plus expressions: valid and invalid
LEGAL_PLUS:    LOAD LABEL2+3
ILLEGAL_PLUS:  LOAD 2+X      ; right side not numeric

; LABEL2 defined later
LABEL2:        SPACE 1

; ---------- Valid lines for contrast ----------
OK1:           MOV A, B
ok2:           ADD A, B

; end of tests
