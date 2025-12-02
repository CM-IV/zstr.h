
#include <stdio.h>
#include "zstr.h"

int main(void) 
{
    printf("=> UTF-8 test\n\n");

    zstr s = zstr_from("Hi \xF0\x9F\x8C\x8D"); 

    printf("Input: '%s'\n", zstr_cstr(&s));
    
    if (zstr_is_valid_utf8(&s)) 
    {
        printf("[PASS] String is valid UTF-8.\n");
    } 
    else 
    {
        printf("[FAIL] String marked invalid!\n");
    }

    size_t bytes = zstr_len(&s);
    size_t runes = zstr_count_runes(&s);

    printf("Bytes: %zu\n", bytes); // Should be 3 + 4 = 7
    printf("Runes: %zu\n", runes); // Should be 3 + 1 = 4

    printf("\nDecoded Runes:\n");
    const char *it = zstr_cstr(&s);
    while (*it) 
    {
        uint32_t r = zstr_next_rune(&it);
        printf(" U+%04X", r);
    }
    printf("\n\n");

    zstr bad = zstr_from("\xC0\x80"); 
    
    printf("Input Bad: [C0 80]\n");
    if (!zstr_is_valid_utf8(&bad)) 
    {
        printf("[PASS] Invalid sequence correctly rejected.\n");
    } 
    else 
    {
        printf("[FAIL] Invalid sequence accepted!\n");
    }

    zstr_free(&s);
    zstr_free(&bad);

    return 0;
}
