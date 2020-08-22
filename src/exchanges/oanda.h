#ifndef EXCHANGES_OANDA_H
#define EXCHANGES_OANDA_H

#include <ffjson/ffjson.h>
#include <pprint.h>

/*
 * Starts an oanda exchange feed
 *
 * @param key the key needed to connect to oanda
 */
void exchanges_oanda_init(char *key);

#endif
