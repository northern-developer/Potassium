grammar ptasmMachine;

/* The small languages inside a machine file's strings (plan 0.2:E5.T3).
   ANTLR4 ver 4.10 or newer required. One lexer and one set of shared
   rules, with an entry rule per language; the loader calls the entry
   rule its JSON key needs. Which word may stand where (reg, imm, a
   micro-op's name) is the loader's check, not the grammar's, so a
   misspelled word gets a message that names it. Whitespace is skipped;
   the loader warns when a string is spelled untidily, and uses the
   standard spelling internally. */

// ---- Entry rules

// An operand pattern: a form's key, a mode's name, an operand set's
// member, or a pseudo-instruction's operands. "", "reg, imm|local|reloc",
// "rd:reg, [reg + %lo(local)]".
operandPattern
    : (patternOperand (',' patternOperand)*)? EOF
    ;

// An instruction's or a mode set's semantics:
// "set(rD, add(rA, rB)); if(F, set(PC, n26))".
semantics
    : (statement (';' statement)* ';'?)? EOF
    ;

// A becomes alternative's condition: "fits(imm, 12)".
condition
    : call EOF
    ;

// A becomes expansion, in Intel operand order:
// "LUI rd, %hi(imm); ADDI rd, rd, %lo(imm)".
expansion
    : asmInstruction (';' asmInstruction)* ';'? EOF
    ;

// ---- Operand patterns

// rd:reg -- a named operand, for a pseudo-instruction's operands.
patternOperand
    : (NAME ':')? patternMode
    ;

patternMode
    : patternWord                          // reg, imm|local|reloc, mem
    | '[' patternAddress ']'               // [reg + imm]
    | '[' '[' patternAddress ']' ']'       // [[pc + imm]]
    ;

// reg + reg * imm + imm: base, index times scale, displacement.
patternAddress
    : patternWord ('+' patternWord ('*' patternWord)?)*
    ;

patternWord
    : addressPart
    | alternatives
    ;

alternatives
    : NAME ('|' NAME)*
    ;

// %hi(local); OR1K's lo(imm).
addressPart
    : '%'? NAME '(' alternatives ')'
    ;

// ---- Semantics and conditions: every operation a call

// set(...), store.32(...), if(...), trap(...).
statement
    : call
    ;

call
    : callee '(' (value (',' value)*)? ')'
    ;

// load.u32, sext.8; {op}, filled from semArgs.
callee
    : dottedName
    | placeholder
    ;

value
    : call
    | '-'? NUMBER
    | dottedName                           // rD, CY, pc.next, xlen
    | OPERAND                              // $2
    | placeholder                          // {cy}
    ;

placeholder
    : '{' NAME '}'
    ;

// load.u32, rotr.17, pc.next.
dottedName
    : NAME ('.' (NAME | NUMBER))*
    ;

// ---- Becomes text

asmInstruction
    : mnemonic (asmOperand (',' asmOperand)*)?
    ;

// ADDI, C.LI, l.add, FENCE.I.
mnemonic
    : NAME ('.' NAME)*
    ;

asmOperand
    : asmValue
    | '[' asmAddress ']'
    | '[' '[' asmAddress ']' ']'
    ;

asmAddress
    : asmValue (('+' | '-') asmValue ('*' asmValue)?)*
    ;

asmValue
    : '%'? NAME '(' asmValue ')'           // %hi(imm)
    | '-'? NUMBER
    | NAME                                 // rd, x0, imm
    ;

// ---- The one lexer

NAME
    : [A-Za-z_] [A-Za-z0-9_]*
    ;

NUMBER
    : '0' [xX] [0-9A-Fa-f]+
    | '0' [bB] [01]+
    | [0-9]+
    ;

OPERAND
    : '$' [0-9]+
    ;

WS
    : [ \t\r\n]+ -> skip
    ;
