# Seed corpus

Empty on purpose. `fuzz_greeter.cpp` (and `ai1::greet` with it) was
removed when the real ptasm parser/codegen was merged in -- there
is now a real lexer/parser entry point (`ptasmLexer`/`ptasmParser`)
worth fuzzing.

Once a harness over that entry point exists, seed this directory with
small valid programs (see `../../examples/`) so the fuzzer starts near
interesting code instead of from nothing.
