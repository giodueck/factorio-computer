#ifndef BASE64_H
#define BASE64_H

int b64_get_value(char c);

int b64_encoded_size(int inlen);

int b64_decoded_size(int inlen);

char* b64_encode(const unsigned char* in, int len);

char* b64_decode(const unsigned char* in, int len);

char *bps_decode(const unsigned char *in, int len);

int bps_to_json(const unsigned char *filename_in, const unsigned char *filename_out);

int bps_to_json_stdout(const unsigned char *filename_in);

#endif // BASE64_H