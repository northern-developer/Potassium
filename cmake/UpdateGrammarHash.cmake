# Invoked via `cmake -P` from the regen-parser custom target, after
# regeneration succeeds. Expects ANTLR_GRAMMAR, ANTLR_GRAMMAR_HASH_FILE
# on the command line (-D).
file(SHA256 "${ANTLR_GRAMMAR}" GRAMMAR_HASH)
file(WRITE "${ANTLR_GRAMMAR_HASH_FILE}" "${GRAMMAR_HASH}\n")
