#include "base_conversion.h"

uint64_t
btctosat_str(char *num)
{
  long double btc = strtold(num, NULL);
  btc = btc * BTC_TO_SATOSHI_SCALE;

  uint64_t satoshi = (uint64_t)btc;
  return satoshi;
}

uint64_t
usdtocent_str(char *num)
{
  long double usd = strtold(num, NULL);
  usd = usd * USD_TO_CENTS_SCALE;

  uint64_t cents = (uint64_t)usd;
  return cents;
}
