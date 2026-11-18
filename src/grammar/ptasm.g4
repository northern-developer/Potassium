grammar ptasm;

/* ANTLR4 ver 4.10 or newer required */
options {
    caseInsensitive = true;
}

// Two top-level entry rules, not one -- ROADMAP.md's Ticket 4 "whole-file
// grammar dispatch mechanism": which one gets called is decided once per
// run, before parsing starts (by the resolved cpu.json's declared syntax),
// not per line. `intelLine`/`attLine` both reference the *same*
// `directive`/`dataDirective`/`label` rules, unchanged -- those are
// already shared between conventions; only `instruction` (the mnemonic +
// operand-list shape) actually differs, so only it gets an AT&T-specific
// sibling (`attInstruction`, below).
intelProg
    : intelLine* intelLastLine? EOF
    ;

attProg
    : attLine* attLastLine? EOF
    ;

// A line may start with whitespace, labelled or not: the `:` is what
// marks a label, so `  y: AAA` is label `y` then instruction `AAA`.
intelLine
    : (
        instruction
        | WS? directive
        | (WS? label)? WS? dataDirective
        | WS? label
    )? WS? COMMENT? EOL
    ;

attLine
    : (
        attInstruction
        | WS? directive
        | (WS? label)? WS? dataDirective
        | WS? label
    )? WS? COMMENT? EOL
    ;

// The file's last line, which need not end with a newline. Never empty,
// so the loop above and this rule cannot both claim the same input.
intelLastLine
    : (
        instruction
        | WS? directive
        | (WS? label)? WS? dataDirective
        | WS? label
    ) WS? COMMENT?
    | WS? COMMENT
    | WS
    ;

attLastLine
    : (
        attInstruction
        | WS? directive
        | (WS? label)? WS? dataDirective
        | WS? label
    ) WS? COMMENT?
    | WS? COMMENT
    | WS
    ;

instruction
    : (WS? label)? WS? mnemonic WS? operandList
    ;

attInstruction
    : (WS? label)? WS? mnemonic WS? attOperandList
    ;

directive
    : name WS EQU WS expression
    | name WS SET WS expression
    | ORG WS expression
    | GLOBAL WS name
    | ALIGN WS expression
    | EXTERN WS name
    | SECTION WS sectionName
    | TEXT
    | DATA
    | BSS
    ;

// A section name may be conventional and dotted (.data) or plain (data).
sectionName
    : name
    | SECTION_NAME
    ;


label
    : name ':'
    ;

operandList
    : (operand WS? ( ',' WS? operand )*)?
    ;

// A register is a name (E1.T1): R<n>, PC, or a name in the machine
// file's register list. The grammar can't tell a register from a label,
// so an operand is a value, or a bracketed address with its terms in any
// order; AsmParser sets its mode from which terms are registers.
operand
    : value
    | memory
    | memoryIndirect
    ;

memory
    : '[' WS? address WS? ']'
    ;

memoryIndirect
    : '[' WS? '[' WS? address WS? ']' WS? ']'
    ;

// [BX + 4], [4 + BX], [msg + BX], [R10 + %lo(msg)]: each term with the
// sign before it.
address
    : addressTerm (WS? ('+' | '-') WS? addressTerm)*
    ;

addressTerm
    : '-'? WS? (addressPart | registerTerm | name | number)
    ;

// An operand's number, or one part of an address written PART(expr):
// %hi(msg) and %lo(msg) (RISC-V), hi(msg), ha(msg) and lo(msg) (OR1K).
// Which parts exist is up to the cpu.json's forms.
value
    : addressPart
    | expression
    ;

addressPart
    : '%'? name WS? '(' WS? expression WS? ')'
    ;

// R<n> and PC are always registers; a list name is a `name`, told apart
// in AsmParser.
registerTerm
    : REG
    | PC
    ;

// A sum of names and numbers; the first may be negated (`-16`, `-msg + 4`).
// A register here is a register operand when it stands alone, and an
// error anywhere else (registers aren't values).
expression
    : '-'? WS? (name | number | registerTerm)
    | expression WS? ( '+'| '-') WS? (name | number | registerTerm)
    ;

// AT&T addressing modes (ROADMAP.md's Ticket 4 "addressing-mode
// decidability" section) -- a separate rule set from Intel's own above,
// not a shared one: every real shape here has its own unambiguous
// leading token (`%`/`$`/`(`/a bare `expression`), so no grammar-level
// ambiguity between alternatives, but the two conventions' *shapes*
// (bracket-with-plus vs. displacement-before-parens, for one) are
// genuinely different, not just re-spelled -- see the GAS-vs-MASM
// addressing-mode reference table in ROADMAP.md. `REG` itself is a
// shared leaf, unchanged from Intel's own `register`/`registerIndirect`
// above -- only the sigil/bracket shape wrapping it differs.
attOperandList
    : (attOperand WS? ( ',' WS? attOperand )*)?
    ;

attOperand
    : attRegister
    | attImmediate
    | attIndirectTarget
    | attScaledIndex
    | attIndexed
    | attRegisterIndirect
    | value
    ;

// A displacement may lead with a minus, as any expression may: `-4(%R5)`.
attDisplacement
    : value
    ;

// %R5, %PC, or %name from the register list (E1.T1 decision 7).
attReg
    : '%' (REG | PC | NAME)
    ;

attRegister
    : attReg
    ;

attRegisterIndirect
    : '(' WS? attReg WS? ')'
    ;

attImmediate
    : '$' value
    ;

// `disp(%reg)`; with the program counter (`disp(%rip)`), PC-relative,
// Intel's `[RIP + disp]`.
attIndexed
    : attDisplacement WS? '(' WS? attReg WS? ')'
    ;

// `disp(base, index, scale)` -- the one real mode with no single ptasm
// ArgType to map onto (see the decidability section's own resolution):
// the Visitor needs to turn one match of this rule into *two* ArgData
// entries (base in one operand slot, index-plus-scale in another),
// not implemented yet -- this is the grammar shape alone, matching the
// real AT&T form exactly (displacement, base register, index register,
// and a constant scale, in that order).
attScaledIndex
    : attDisplacement WS? '(' WS? attReg WS? ',' WS? attReg WS? ',' WS? number WS? ')'
    ;

// `*%reg`: AT&T's explicit indirect jump/call target marker -- not its
// own ArgType, resolves to the same `regindex` meaning as a bare
// `(%reg)` (see the reference table) once the Visitor-side mapping for
// it exists.
attIndirectTarget
    : '*' WS? attReg
    ;

string_
    : STRING
    ;

name
    : NAME
    ;

number
    : HEX
    | BYTE
    | NUMBER
    ;

mnemonic
    : NAME
    ;
    
dataDirective
    : DATA_BYTE WS byteList
    | DATA_STRING WS STRING
    | DATA_STRINGZ WS STRING
    | DATA_WORD WS expressionList
    | DATA_LONG WS expressionList
    | DATA_QUAD WS expressionList
    | DATA_ADDR WS expressionList
    | SPACE WS expression
    ;

byteList
    : BYTE ( WS? ',' WS? BYTE )*
    ;

// .WORD/.LONG/.INT/.QUAD/.ADDR's items: expressions, labels included
// (Ticket 20), each resolved like an instruction operand.
expressionList
    : expression ( WS? ',' WS? expression )*
    ;

ORG
    : '.ORG'
    ;

EQU
    : '.EQU'
    ;

// Like EQU, but may be redefined; a use sees the value set before it.
SET
    : '.SET'
    ;

GLOBAL
    : '.GLOBAL'
    | '.GLOBL'
    ;

ALIGN
    : '.ALIGN'
    ;

// Recorded via CodeGen::markExtern/isExtern (mirroring .GLOBAL's own
// markGlobal/isGlobal) -- purely a record for now, same as .GLOBAL was
// before the ELF writer existed to consult it. Not yet wired into any
// "referenced but never defined" error, since that mechanism doesn't
// exist yet either (see ROADMAP.md Ticket 4's deferred name-lookup note).
EXTERN
    : '.EXTERN'
    ;

// Starts a new, explicitly-named Block -- or, if the current block is
// still empty, just names it in place rather than leaving an empty block
// behind. See CodeGen::openSection.
SECTION
    : '.SECTION'
    ;

// Shorthands for .SECTION .text / .data / .bss.
TEXT
    : '.TEXT'
    ;

DATA
    : '.DATA'
    ;

BSS
    : '.BSS'
    ;

// Reserves n zero bytes (no file bytes in a .bss section).
SPACE
    : '.SPACE'
    ;

// The program counter on every CPU (E1.T1 decision 17). Other spellings
// of it (AMD64's RIP) are entries in the machine file's register list,
// with the pc role.
PC
    : 'PC'
    ;


DATA_BYTE
    : '.BYTE'
    ;

DATA_STRING
    : '.ASCII'
    ;

// Like DATA_STRING (.ASCII), but Visitor::visitDataline appends a
// trailing NUL -- the one difference between the two, per Ticket 3's
// own directive table ("an ASCII string with an automatic \0
// terminator" vs. "a raw ASCII string, no terminator").
DATA_STRINGZ
    : '.ASCIZ'
    ;

DATA_WORD
    : '.WORD'
    ;

DATA_LONG
    : '.LONG'
    | '.INT'
    ;

DATA_QUAD
    : '.QUAD'
    ;

// An address-sized slot (machine.addressBits, rounded up to a power of
// two bytes); .dc.a is GAS's name for it.
DATA_ADDR
    : '.ADDR'
    | '.DC.A'
    ;

// A dotted section name, e.g. .data or .rodata.str. Defined after every
// '.'-keyword: on an equal-length match ANTLR picks the earlier rule, so
// .ORG and the rest stay keywords.
SECTION_NAME
    : '.' [_A-Z] [_A-Z0-9.]*
    ;

/*
* opcodes
*  .. see NAME ..
*/


REG : 'R'[0-9]+
    ;

NAME
    : [_A-Z] [_A-Z0-9.]*
    ;

NUMBER
    : [0-9]+
    ;

BYTE
    : '0X' [0-9A-F][0-9A-F]
    ;

HEX
    : '0X' [0-9A-F]* /*TODO: MHW  add support HEX and BYTE for 9h and 0ah ('H')? */
    ;

// `#` is GAS/AT&T's own comment char (ROADMAP.md Ticket 4) -- shared
// unconditionally, not gated to AT&T mode: an Intel-syntax file simply
// never happens to use it unless written that way.
COMMENT
    : (';' | '--' | '#') ~ [\r\n]*
    ;

STRING
    : '"' ~ ["]* '"'
    ;

EOL
    : [\r\n]+
    ;

WS
    : [ \t]+
    ;
