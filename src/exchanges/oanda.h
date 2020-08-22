#ifndef EXCHANGES_OANDA_H
#define EXCHANGES_OANDA_H

#include <ffjson/ffjson.h>
#include <httpws/http.h>
#include <pprint.h>

#define OANDA_API_ROOT "api-fxpractice.oanda.com"

/*
 * Starts an oanda exchange feed
 *
 * @param key the key needed to connect to oanda
 */
void exchanges_oanda_init(char *key);

#endif
