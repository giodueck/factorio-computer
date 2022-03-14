#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "base64.h"
#include "zhelpers.h"
#include <json-parser/json.h>
#include <zlib/zlib.h>

int main(int argc, char *argv[])
{
    // BASE 64 DECODE BLUEPRINT STRING INTO A BINARY FILE
    FILE *fp, *fout;
    char *filename;
    struct stat filestatus;
    int file_size;
    char *file_contents;
    char *out;

    if (argc != 2)
    {
        fprintf(stderr, "%s <blueprint_string.txt>\n", argv[0]);
        return 1;
    }
    filename = argv[1];

    if (stat(filename, &filestatus) != 0)
    {
        fprintf(stderr, "File %s not found\n", filename);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char *)malloc(filestatus.st_size);
    if (file_contents == NULL)
    {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fp = fopen(filename, "rt");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    if (fread(file_contents, file_size, 1, fp) != 1)
    {
        fprintf(stderr, "Unable to read content of %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    fclose(fp);

    out = bps_decode(file_contents, file_size);
    fout = fopen("output", "wb");
    fwrite(out, 1, b64_decoded_size(file_size - 1), fout);
    fclose(fout);
    free(file_contents);

    // INFLATE DECODED BINARY
    int ret;

    /* do decompression */
    fout = fopen("output", "rb");
    ret = inf(fout, stdout);
    if (ret != Z_OK)
        zerr(ret);
    fclose(fout);
    remove("output");
    return ret;
}