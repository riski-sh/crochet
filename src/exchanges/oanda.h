#ifndef EXCHANGES_OANDA_H
#define EXCHANGES_OANDA_H

#include <exchanges/exchanges.h>
#include <ffjson/ffjson.h>
#include <globals/globals.h>
#include <httpws/http.h>
#include <pprint/pprint.h>
#include <security/security.h>

#define OANDA_API_ROOT "api-fxpractice.oanda.com"

#define OANDA_PRINT_NTERVAL_SECONDS 5.0
/*
 * Starts an oanda exchange feed
 *
 * @param key the key needed to connect to oanda
 */
void exchanges_oanda_init(char *key);

#endif
