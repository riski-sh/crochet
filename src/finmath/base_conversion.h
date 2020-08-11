#ifndef BASE_CONVERSION_H
#define BASE_CONVERSION_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BTC_TO_SATOSHI_SCALE 100000000
#define USD_TO_CENTS_SCALE 100

/*
 * Converts a bitcoin value into its satoshi equivalent
 */
uint64_t btctosat_str(char *num);

/*
 * Converts a USD dollar amount into its cents equivalent
 */
uint64_t usdtocent_str(char *num);

#endif
