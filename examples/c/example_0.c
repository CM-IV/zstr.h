
#include <stdio.h>
#include <string.h>
#include "zstr.h"

int main(void) 
{
    printf("=> HTTP Header Parsing (zero-copy)\n\n");

    zstr raw_input = zstr_from("   Authorization:   Bearer TOKEN_XYZ_123   ");

    zstr_view line = zstr_as_view(&raw_input);
    line = zstr_view_trim(line);

    printf("Processing: '%.*s'\n", (int)line.len, line.data);

    if (zstr_view_starts_with(line, "Authorization:")) 
    {
        //    "Authorization:" is 14 chars long.
        //    We slice from index 14 to the end.
        zstr_view value = zstr_sub(line, 14, line.len - 14);
        
        value = zstr_view_trim(value);

        if (zstr_view_starts_with(value, "Bearer "))
        {
            // Slice past "Bearer " (7 chars)
            zstr_view token = zstr_sub(value, 7, value.len - 7);
            
            printf("  [x] Auth Type: Bearer\n");
            printf("  [x] Token:     '%.*s'\n", (int)token.len, token.data);
            
            // Validation example, this is just for the example.
            if (zstr_view_eq(token, "TOKEN_XYZ_123")) 
            {
                printf("  [x] Validation: Access Granted.\n");
            }
        } 
        else 
        {
            printf("  [!] Unknown Auth Type.\n");
        }

    } 
    else if (zstr_view_starts_with(line, "Content-Type:")) 
    {
        printf("  [i] Found Content-Type header.\n");
    } 
    else 
    {
        printf("  [?] Unknown Header.\n");
    }

    zstr_free(&raw_input);
    return 0;
}