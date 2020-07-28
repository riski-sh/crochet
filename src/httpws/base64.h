#ifndef BASE64_H
#define BASE64_H

#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char *base64(const unsigned char *input, int length);
unsigned char *decode64(const unsigned char *input, int length);

#endif
