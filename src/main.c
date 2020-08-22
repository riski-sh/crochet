/*
 * Crochet
 * An open source crypto currency router and agregated feed broker.
 * (C) washcloth et al.
 */

#include "ffjson/ffjson.h"
#if defined(DO_UNIT_TESTS)
#include <orderbooks/book.h>
#include <stdlib.h>
int
main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  UNIT_TEST_DO(book_query)

  return 0;
}

#else

#include <exchanges/exchanges.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <orderbooks/book.h>
#include <stdio.h>

#include "pprint.h"

static __json_value
_load_config(char *file, char **raw)
{
  FILE *fp = fopen(file, "r");
  fseek(fp, 0, SEEK_END);
  size_t file_size = (size_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *cfg = malloc(file_size + 1);
  fread(cfg, sizeof(char), file_size, fp);
  cfg[file_size] = '\x0';

  *raw = cfg;

  fclose(fp);
  return json_parse(cfg);
}

int
main(int argc, char **argv)
{

  pprint_info(
      "crochet (C) washcloth et al.", __FILE_NAME__, __func__, __LINE__);

  pprint_info("loading configuration file", __FILE_NAME__, __func__, __LINE__);

  __json_value _config_root = NULL;
  __json_object config = NULL;
  char *_config_raw = NULL;
  if (argc == 2) {
    _config_root = _load_config(argv[1], &_config_raw);
    config = json_get_object(_config_root);
  } else {
    pprint_error("please specify a configuration file", __FILE_NAME__, __func__,
        __LINE__);
    return 1;
  }

  pprint_info("initializing OpenSSL", __FILE_NAME__, __func__, __LINE__);

  SSL_load_error_strings();
  SSL_library_init();

  __json_object _oanda = json_get_object(hashmap_get("oanda", config));
  if (_oanda != NULL) {
    __json_bool online = json_get_bool(hashmap_get("online", _oanda));
    if (online) {
      __json_string key = json_get_string(hashmap_get("key", _oanda));
      pprint_info(
          "starting oanda exchange feed", __FILE_NAME__, __func__, __LINE__);
      exchanges_oanda_init(key);
    }
  }

  __json_object _coinbase = json_get_object(hashmap_get("coinbase", config));
  if (_coinbase != NULL) {
    __json_bool online = json_get_bool(hashmap_get("online", _coinbase));
    if (online) {
      pprint_info(
          "starting coinbase exchange feed", __FILE_NAME__, __func__, __LINE__);
      exchanges_coinbase_init();
    }
  }

  free(_config_raw);
  json_free(_config_root);

  return 0;
}
#endif
