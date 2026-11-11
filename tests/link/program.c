typedef __INTPTR_TYPE__ word;
int puts(const char *);
long write(int, const void *, unsigned long);
const char *ptasm_words[] = { "From", "a table.", &"Goodbye!"[4] };
const unsigned short ptasm_numbers[] = { 0x1234, 7 };
word ptasm_calls;
__attribute__((noinline)) static void goodbye(void) { puts("Goodbye!"); }
void ptasm_program(word x) {
    ++ptasm_calls;
    write(1, "Hello, World!\n", 14);
    if (x % 2 == 1) {
        goodbye();
    }
}
