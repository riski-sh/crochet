#ifndef COINBASE_H
#define COINBASE_H

#include <ffjson/ffjson.h>
#include <httpws/http.h>
#include <pprint.h>

/*
 * The base url that will serve as the root for
 * https/wss connections.
 */
#define COINBASE_URL "api.pro.coinbase.com"

void coinbase_init(void);

#endif
