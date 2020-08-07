#include "base64.h"

unsigned char *
base64(const unsigned char *input, int length)
{
  const long pl = (unsigned)(4 * ((length + 2) / 3));
  unsigned char *output = calloc((unsigned long)pl + 1,
      1); //+1 for the terminating null that EVP_EncodeBlock adds on
  const int ol = EVP_EncodeBlock(output, input, length);
  if (ol != pl) {
    fprintf(stderr, "Whoops, encode predicted %lu but we got %d\n", pl, ol);
  }
  return output;
}

unsigned char *
decode64(const unsigned char *input, int length)
{
  const int pl = 3 * length / 4;
  unsigned char *output = calloc((unsigned long)pl + 1, 1);
  const int ol = EVP_DecodeBlock(output, input, length);
  if (pl != ol) {
    fprintf(stderr, "Whoops, decode predicted %d but we got %d\n", pl, ol);
  }
  return output;
}
