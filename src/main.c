/*
 * Crochet
 * An open source crypto currency router and agregated feed broker.
 * (C) washcloth et al.
 */

#include <exchanges/coinbase.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <orderbooks/book.h>
#include <stdio.h>

#include "pprint.h"

int
main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  // pprint_info(
  //    "crochet (C) washcloth et al.", __FILE_NAME__, __func__, __LINE__);

  // pprint_info("initializing OpenSSL", __FILE_NAME__, __func__, __LINE__);

  SSL_load_error_strings();
  SSL_library_init();

  coinbase_init();

  return 0;
}
