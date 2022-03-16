#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include <json-parser/json.h>

const char *commands[2] = {"decode", "encode"};

int main(int argc, char *argv[])
{
    char *filename_in, *filename_out;
    int ret;

    if (argc < 3)
    {
        // fprintf(stderr, "%s <blueprint_string.txt> [optional <output.json>]\n", argv[0]);
        fprintf(stderr, "%s <command> [command args]\n", argv[0]);
        return 1;
    }
    filename_in = argv[2];

    if (strcmp(argv[1], commands[0]) == 0)
    {
        if (argc > 3)
        {
            filename_out = argv[3];
            bps_to_json(filename_in, filename_out);
        }
        else
            bps_to_json_stdout(filename_in);
    } else if (strcmp(argv[1], commands[1]) == 0)
    {
        if (argc > 3)
        {
            filename_out = argv[3];
            json_to_bps(filename_in, filename_out);
        }
        else
            json_to_bps_stdout(filename_in);
    }
    return 0;
}