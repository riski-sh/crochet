/*
 * Crochet
 * (C) washcloth et al.
 */

#include <exchanges/exhangesall.h>
#include <ffjson/ffjson.h>
#include <globals/globals.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <orderbooks/book.h>
#include <pprint/pprint.h>
#include <pthread.h>
#include <security/analysis.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

static __json_value
_load_config(char *file, char **raw)
{
  FILE *fp = fopen(file, "r");

  if (!fp)
  {
    pprint_error("unable to open config file at %s (aborting)", file);
    abort();
  }

  pprint_info("loading config file at %s", file);

  fseek(fp, 0, SEEK_END);
  size_t file_size = (size_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *cfg = malloc(file_size + 1);
  size_t ret = fread(cfg, sizeof(char), file_size, fp);
  if (ret > 0 && (size_t)ret != file_size)
  {
    pprint_error("unable to read entire config file? (aborting)", __LINE__);
    abort();
  }
  cfg[file_size] = '\x0';

  *raw = cfg;

  fclose(fp);
  return json_parse(cfg);
}

static void
sig_handler(int sig)
{
  if (sig == SIGINT)
  {
    printf("\r");
    pprint_warn("%s", "<CTRL>+C SIGINT");
    bool disable = false;
    globals_continue(&disable);
  }
  else
  {
    printf("\r");
    pprint_error("signal %d (aborting)", sig);
    abort();
  }
}

int
main(int argc, char **argv)
{
  exchange_init();

  signal(SIGINT, sig_handler);

  __json_value _config_root = NULL;
  __json_object config = NULL;
  char *_config_raw = NULL;
  if (argc == 2)
  {
    _config_root = _load_config(argv[1], &_config_raw);
    config = json_get_object(_config_root);
  }
  else
  {
    pprint_error("%s", "no configuration file specified");
    return 1;
  }

  pprint_info("%s", "loading analysis");
  analysis_init("./obj/libs/");
  pprint_info("%s", "setting up openssl");

  SSL_load_error_strings();
  ERR_load_crypto_strings();
  SSL_library_init();

  pthread_t oanda_mainloop = pthread_self();

  __json_object _oanda = json_get_object(hashmap_get("oanda", config));
  if (_oanda != NULL)
  {
    __json_bool online = json_get_bool(hashmap_get("online", _oanda));
    if (online)
    {
      __json_string key = json_get_string(hashmap_get("key", _oanda));
      pprint_info("starting oanda feed with api key [REDACTED]", key);

      if (pthread_create(&oanda_mainloop, NULL, &exchanges_oanda_init, key))
      {
        fprintf(stderr, "Error creating thread\n");
        return 1;
      }
    }
  }

  __json_object _coinbase = json_get_object(hashmap_get("coinbase", config));
  if (_coinbase != NULL)
  {
    __json_bool online = json_get_bool(hashmap_get("online", _coinbase));
    if (online)
    {
      pprint_info("%s", "starting coinbase feed");
      abort();
    }
  }

  if (oanda_mainloop != pthread_self())
  {
    pthread_join(oanda_mainloop, NULL);
    pprint_info("%s", "joined oanda_mainloop");
  }

  pprint_info("%s", "cleaning up main...");

  free(_config_raw);
  json_free(_config_root);
  exchange_free();
  analysis_clear();

  pprint_info("%s", "goodbye");
  return 0;
}
