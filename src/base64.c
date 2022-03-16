#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib/zlib.h>

#include "base64.h"
#include "zhelpers.h"

const char tempfilename[] = "output.tmp";
const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int b64_encoded_size(int inlen)
{
    int ret = inlen;
    if (inlen % 3)
        ret += 3 - inlen % 3;

    ret /= 3;
    ret *= 4;

    return ret;
}

int b64_decoded_size(int inlen)
{
    int ret = inlen;
    ret /= 4;
    ret *= 3;
    return ret;
}

char *b64_encode(const unsigned char *in, int len)
{
    char *out;
    int elen;
    int j = 0;
    int triplet;
    int pad;

    if (in == NULL || len == 0)
        return NULL;

    elen = b64_encoded_size(len);
    out = (char *)malloc(elen * sizeof(char) + 1);
    out[elen] = '\0';

    for (int i = 0; i < len; i += 3)
    {
        triplet = in[i] << 16;
        pad = 0;
        if ((i + 1) < len)
        {
            triplet += in[i + 1] << 8;
            if ((i + 2) < len)
                triplet += in[i + 2];
            else
                pad = 1;
        }
        else
            pad = 2;

        out[j++] = b64_chars[(triplet >> 18) & 63];
        out[j++] = b64_chars[(triplet >> 12) & 63];
        if (pad == 2)
            out[j++] = '=';
        else
            out[j++] = b64_chars[(triplet >> 6) & 63];
        if (pad >= 1)
            out[j++] = '=';
        else
            out[j++] = b64_chars[triplet & 63];
    }

    return out;
}

int b64_get_value(char c)
{
    for (int i = 0; i < 64; i++)
    {
        if (c == b64_chars[i])
            return i;
    }
    return 0;
}

char *b64_decode(const unsigned char *in, int len)
{
    char *out;
    int dlen;
    int quartet;
    int j = 0;

    if (in == NULL || len == 0)
        return NULL;

    dlen = b64_decoded_size(len);
    out = (char *)malloc(dlen * sizeof(char) + 1);

    for (int i = 0; i < len; i += 4)
    {
        quartet = b64_get_value(in[i]) << 18;
        quartet += b64_get_value(in[i + 1]) << 12;
        quartet += b64_get_value(in[i + 2]) << 6;
        quartet += b64_get_value(in[i + 3]);

        out[j++] = (quartet >> 16) & 255;
        out[j++] = (quartet >> 8) & 255;
        out[j++] = quartet & 255;
    }
    out[j] = '\0';

    return out;
}

char *bps_decode(const unsigned char *in, int len)
{
    return b64_decode(in + sizeof(char), len - 1);
}

char *bps_encode(const unsigned char *in, int len)
{
    char *encoded = b64_encode(in, len);
    char *encoded_bps;

    encoded_bps = (char*)malloc((strlen(encoded) + 1) * sizeof(char));
    encoded_bps[0] = VERSION_BYTE;
    encoded_bps[1] = '\0';
    strcat(encoded_bps, encoded);
    free(encoded);
    return encoded_bps;
}

int bps_to_json(const unsigned char *filename_in, const unsigned char *filename_out)
{
    FILE *fp, *fouttmp, *fout;
    struct stat filestatus;
    int file_size;
    char *file_contents;
    char *out;
    int ret;

    // BASE 64 DECODE BLUEPRINT STRING INTO A BINARY FILE
    if (stat(filename_in, &filestatus) != 0)
    {
        fprintf(stderr, "File %s not found\n", filename_in);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char *)malloc(filestatus.st_size);
    if (file_contents == NULL)
    {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fp = fopen(filename_in, "rt");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename_in);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    if (fread(file_contents, file_size, 1, fp) != 1)
    {
        fprintf(stderr, "Unable to read content of %s\n", filename_in);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    fclose(fp);

    out = bps_decode(file_contents, file_size);
    fouttmp = fopen(tempfilename, "wb");
    fwrite(out, 1, b64_decoded_size(file_size - 1), fouttmp);
    fclose(fouttmp);
    free(file_contents);
    free(out);

    // INFLATE DECODED BINARY
    fouttmp = fopen(tempfilename, "rb");
    if (fouttmp == NULL)
    {
        fprintf(stderr, "Unable to create temporary file %s\n", tempfilename);
        fclose(fouttmp);
        return 1;
    }
    fout = fopen(filename_out, "wb");
    if (fout == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename_out);
        fclose(fout);
        fclose(fouttmp);
        remove(tempfilename);
        return 1;
    }
    ret = inf(fouttmp, fout);
    if (ret != Z_OK)
        zerr(ret);
    fclose(fouttmp);
    remove(tempfilename);
    return ret;
}

int bps_to_json_stdout(const unsigned char *filename_in)
{
    FILE *fp, *fouttmp;
    struct stat filestatus;
    int file_size;
    char *file_contents;
    char *out;
    int ret;

    // BASE 64 DECODE BLUEPRINT STRING INTO A BINARY FILE
    if (stat(filename_in, &filestatus) != 0)
    {
        fprintf(stderr, "File %s not found\n", filename_in);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char *)malloc(filestatus.st_size);
    if (file_contents == NULL)
    {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fp = fopen(filename_in, "rt");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename_in);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    if (fread(file_contents, file_size, 1, fp) != 1)
    {
        fprintf(stderr, "Unable to read content of %s\n", filename_in);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    fclose(fp);

    out = bps_decode(file_contents, file_size);
    fouttmp = fopen(tempfilename, "wb");
    if (fouttmp == NULL)
    {
        fprintf(stderr, "Unable to create temporary file %s\n", tempfilename);
        fclose(fouttmp);
        free(file_contents);
        return 1;
    }
    fwrite(out, 1, b64_decoded_size(file_size - 1), fouttmp);
    fclose(fouttmp);
    free(file_contents);
    free(out);

    // INFLATE DECODED BINARY
    fouttmp = fopen(tempfilename, "rb");
    if (fouttmp == NULL)
    {
        fprintf(stderr, "Unable to open temporary file %s\n", tempfilename);
        fclose(fouttmp);
        return 1;
    }
    ret = inf(fouttmp, stdout);
    if (ret != Z_OK)
        zerr(ret);
    fclose(fouttmp);
    remove(tempfilename);
    return ret;
}

int json_to_bps(const unsigned char *filename_in, const unsigned char *filename_out)
{
    FILE *fp, *fouttmp, *fout;
    struct stat filestatus;
    int ret;
    char *file_contents, *out;
    int file_size;

    // DEFLATE JSON FILE
    if (stat(filename_in, &filestatus) != 0)
    {
        fprintf(stderr, "File %s not found\n", filename_in);
        return 1;
    }
    fp = fopen(filename_in, "rt");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename_in);
        fclose(fp);
        return 1;
    }

    fouttmp = fopen(tempfilename, "wb");
    if (fouttmp == NULL)
    {
        fprintf(stderr, "Unable to create temporary file %s\n", tempfilename);
        fclose(fouttmp);
        fclose(fp);
        return 1;
    }

    ret = def(fp, fouttmp, 9);
    if (ret != Z_OK)
    {
        zerr(ret);
        return ret;
    }
    fclose(fouttmp);
    fclose(fp);

    // B64 ENCODE RESULTING BINARY
    if (stat(tempfilename, &filestatus) != 0)
    {
        fprintf(stderr, "File %s not found\n", tempfilename);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char *)malloc(filestatus.st_size);
    fouttmp = fopen(tempfilename, "rb");
    if (fouttmp == NULL)
    {
        fprintf(stderr, "Unable to open temporary file %s\n", tempfilename);
        fclose(fouttmp);
        return 1;
    }
    if (fread(file_contents, file_size, 1, fouttmp) != 1)
    {
        fprintf(stderr, "Unable to read content of %s\n", tempfilename);
        fclose(fouttmp);
        free(file_contents);
        return 1;
    }
    fclose(fouttmp);

    out = bps_encode(file_contents, file_size);
    fout = fopen(filename_out, "wt");
    fwrite(out, 1, b64_encoded_size(file_size) + 1, fout);
    fclose(fout);
    free(file_contents);
    free(out);
    remove(tempfilename);
}

int json_to_bps_stdout(const unsigned char *filename_in)
{
    FILE *fp, *fouttmp, *fout;
    struct stat filestatus;
    int ret;
    char *file_contents, *out;
    int file_size;

    // DEFLATE JSON FILE
    if (stat(filename_in, &filestatus) != 0)
    {
        fprintf(stderr, "File %s not found\n", filename_in);
        return 1;
    }
    fp = fopen(filename_in, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename_in);
        fclose(fp);
        return 1;
    }

    fouttmp = fopen(tempfilename, "wb");
    if (fouttmp == NULL)
    {
        fprintf(stderr, "Unable to create temporary file %s\n", tempfilename);
        fclose(fouttmp);
        fclose(fp);
        return 1;
    }

    ret = def(fp, fouttmp, 9);
    if (ret != Z_OK)
    {
        zerr(ret);
        return ret;
    }
    fclose(fouttmp);
    fclose(fp);

    // B64 ENCODE RESULTING BINARY
    if (stat(tempfilename, &filestatus) != 0)
    {
        fprintf(stderr, "File %s not found\n", tempfilename);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char *)malloc(filestatus.st_size);
    fouttmp = fopen(tempfilename, "rb");
    if (fouttmp == NULL)
    {
        fprintf(stderr, "Unable to open temporary file %s\n", tempfilename);
        fclose(fouttmp);
        return 1;
    }
    if (fread(file_contents, file_size, 1, fouttmp) != 1)
    {
        fprintf(stderr, "Unable to read content of %s\n", tempfilename);
        fclose(fouttmp);
        free(file_contents);
        return 1;
    }
    fclose(fouttmp);

    out = bps_encode(file_contents, file_size);
    printf("%s\n", out);
    free(file_contents);
    free(out);
    remove(tempfilename);
}
