#ifndef EXCHANGES_COINBASE_H
#define EXCHANGES_COINBASE_H

#include <ffjson/ffjson.h>
#include <finmath/base_conversion.h>
#include <globals/globals.h>
#include <orderbooks/coinbase.h>
#include <pprint/pprint.h>
#include <pthread.h>

/*
 * The base url that will serve as the root for
 * https/wss connections.
 */
#define COINBASE_API "api.pro.coinbase.com"

/*
 * Starts the coinbase feeds
 */
void
exchanges_coinbase_init(void);

#endif
