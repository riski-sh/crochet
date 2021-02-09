#ifndef EXCHANGES_OANDA_H
#define EXCHANGES_OANDA_H

#include <exchanges/exchanges.h>
#include <ffjson/ffjson.h>
#include <globals/globals.h>
#include <pprint/pprint.h>
#include <security/chart.h>
#include <security/security.h>
#include <curl/curl.h>

#define OANDA_API_ROOT "https://api-fxtrade.oanda.com"

#define OANDA_PRINT_INTERVAL_SECONDS 1e9

/*
 * Starts an oanda exchange feed
 *
 * @param key the key needed to connect to oanda
 * the type of this key is void to fit the prototype
 * of a pthread
 */
void *
exchanges_oanda_init(void *key);

#endif
