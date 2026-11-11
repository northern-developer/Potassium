-- TESTCPU-0 negative test: deliberately invalid input, to confirm the pipeline
-- 		fails gracefully (a clean "# Not Found" trace line, zero bytes emitted,
-- 		no crash) rather than throwing or corrupting output. See
-- 		tests/test_examples.cpp for the assertions that check both lines below
-- 		actually produce their expected "# Not Found" message.
--
-- Line 1: a mnemonic that doesn't exist in TESTCPU-0.json's Fields at all.
-- Line 2: a real mnemonic (AAB only ever supports mode 1 (reg), one operand)
-- 		used with a mode it has no rule for (mode 2, regindex) -- an existing
-- 		mnemonic paired with an addressing mode it doesn't support.

.ORG 0x0000

ZZZ R0                            -- unsupported mnemonic
AAB [R0]                          -- supported mnemonic, unsupported mode
