#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include <json-parser/json.h>

int main(int argc, char *argv[])
{
    FILE *fout;
    char *filename_in, *filename_out;
    int ret;

    if (argc < 2)
    {
        fprintf(stderr, "%s <blueprint_string.txt> [optional <output.json>]\n", argv[0]);
        return 1;
    }
    filename_in = argv[1];

    if (argc > 2)
    {
        filename_out = argv[2];
        bps_to_json(filename_in, filename_out);
    }
    else
        bps_to_json_stdout(filename_in);
}