#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "json.h"

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        // int len = strlen(argv[1]);
        // printf("%d\n", b64_encoded_size(len));
        // char *b64_str = b64_encode(argv[1], len);
        // printf("%s\n", b64_str);

        int elen = strlen(argv[1]);
        // printf("%d\n", b64_decoded_size(elen));
        char *b64_dec_str = b64_decode(argv[1], elen);
        printf("%s\n", b64_dec_str);

        // free(b64_str);
        free(b64_dec_str);
    }
    return 0;
}