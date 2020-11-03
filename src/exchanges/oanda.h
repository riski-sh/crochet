#ifndef EXCHANGES_OANDA_H
#define EXCHANGES_OANDA_H

#include <client/client.h>
#include <exchanges/exchanges.h>
#include <ffjson/ffjson.h>
#include <globals/globals.h>
#include <httpws/httpws.h>
#include <pprint/pprint.h>
#include <security/security.h>
#include <security/chart.h>

#define OANDA_API_ROOT "api-fxtrade.oanda.com"

#define OANDA_PRINT_NTERVAL_SECONDS 1.0

/*
 * Starts an oanda exchange feed
 *
 * @param key the key needed to connect to oanda
 * the type of this key is void to fit the prototype
 * of a pthread
 */
void *exchanges_oanda_init(void *key);

#endif
