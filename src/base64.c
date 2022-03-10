#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"

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