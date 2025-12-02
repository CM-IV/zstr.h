#include <stdio.h>
#include <stdlib.h>
#include "zstr.h"

// Helper to extract a value from a line ["key": "value" -> value].
zstr_view extract_json_value(zstr_view line)
{
    zstr_split_iter it = zstr_split_init(line, ":");
    zstr_view key_part, val_part;
    
    if (!zstr_split_next(&it, &key_part)) return (zstr_view){"", 0};
    if (!zstr_split_next(&it, &val_part)) return (zstr_view){"", 0};

    val_part = zstr_view_trim(val_part);
    if (zstr_view_ends_with(val_part, ",")) 
    {
        val_part = zstr_sub(val_part, 0, val_part.len - 1);
    }
    
    if (zstr_view_starts_with(val_part, "\"") && zstr_view_ends_with(val_part, "\""))
    {
        val_part = zstr_sub(val_part, 1, val_part.len - 2);
    }

    return val_part;
}

int main(void) 
{
    printf("**zstr JSON Config Loader**\n\n");

    // Returns empty string if file missing.
    zstr config = zstr_read_file("config.json");
    
    if (zstr_is_empty(&config)) 
    {
        fprintf(stderr, "Error: Could not read config.json\n");
        return 1;
    }

    printf("[1] File Loaded (%zu bytes)\n", zstr_len(&config));

    zstr_view full_text = zstr_as_view(&config);
    zstr_split_iter line_iter = zstr_split_init(full_text, "\n");
    zstr_view line;

    zstr server_name = zstr_init();
    int max_conns = 0;

    while (zstr_split_next(&line_iter, &line)) 
    {
        line = zstr_view_trim(line);

        if (zstr_view_starts_with(line, "\"server_name\"")) 
        {
            zstr_view v = extract_json_value(line);
            server_name = zstr_from_view(v);
            printf("  - Found Server Name: " ZSTR_FMT "\n", ZSTR_ARG(server_name));
        }
        else if (zstr_view_starts_with(line, "\"max_connections\"")) 
        {
            zstr_view v = extract_json_value(line);
            // I will modify this later.
            char temp[32];
            snprintf(temp, sizeof(temp), "%.*s", (int)v.len, v.data);
            max_conns = atoi(temp);
            printf("  - Found Max Conns:   %d\n", max_conns);
        }
        else if (zstr_view_starts_with(line, "\"allowed_regions\""))
        {
            zstr_view v = extract_json_value(line);
            printf("  - Found Regions:     %.*s\n", (int)v.len, v.data);
            
            printf("    -> Parsing Regions:\n");
            zstr_split_iter region_iter = zstr_split_init(v, ",");
            zstr_view region;
            while(zstr_split_next(&region_iter, &region)) 
            {
                printf("       * %.*s\n", (int)region.len, region.data);
            }
        }
    }

    printf("\n[2] Modifying Configuration...\n");

    if (zstr_replace(&server_name, "production", "STAGING") == Z_OK)
    {
        printf("  - Environment switched: %s\n", zstr_cstr(&server_name));
    }

    zstr_to_upper(&server_name);
    printf("  - Normalized Name:      %s\n", zstr_cstr(&server_name));

    zstr status_msg = zstr_init();
    zstr_fmt(&status_msg, "Server [%s] is ready accepting %d connections.", 
             zstr_cstr(&server_name), max_conns);
    
    printf("\n[3] Final Status:\n    %s\n", zstr_cstr(&status_msg));

    zstr_free(&config);
    zstr_free(&server_name);
    zstr_free(&status_msg);

    return 0;
}

