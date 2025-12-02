
#include <stdio.h>
#include "zstr.h"

int main(void) 
{
    printf("=> Parsing example\n");

    zstr raw_data = zstr_from("admin, 1001,  Super User  ");
    
    zstr_fmt(&raw_data, ", %s", "ACTIVE"); 

    printf("Parsing: '%s'\n", zstr_cstr(&raw_data));

    zstr_view input = zstr_as_view(&raw_data);
    zstr_split_iter it = zstr_split_init(input, ",");
    zstr_view part;
    
    int field_idx = 0;
    while (zstr_split_next(&it, &part)) 
    {
        part = zstr_view_trim(part);
        
        printf("Field %d: ", field_idx++);
        
        if (field_idx == 1) 
        {
            printf("[User] '%.*s'\n", (int)part.len, part.data);
        } 
        else if (field_idx == 2) 
        {
            // In real code use strtol/atoi on a temp buffer if needed
            printf("[ID]   '%.*s'\n", (int)part.len, part.data);
        }
        else if (zstr_view_eq(part, "ACTIVE")) 
        {
            printf("[Stat] Account is Live!\n");
        }
        else 
        {
            printf("'%.*s'\n", (int)part.len, part.data);
        }
    }

    zstr_free(&raw_data);
    return 0;
}