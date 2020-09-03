#include "security.h"

struct security *
security_new(char *name, int pip_location, int display_precision)
{
  struct security *sec = calloc(1, sizeof(struct security));
  sec->name = strdup(name);
  sec->pip_location = pip_location;
  sec->display_precision = display_precision;
  sec->chart = chart_new();

  return sec;
}

bool
security_update(
    struct security *sec, size_t timestamp, char *best_bid, char *best_ask)
{
  const static int pow10[7] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

  if (timestamp <= sec->last_update) {
    return false;
  }

  /*
   * Convert the string into an equivelent fixed integer
   */
  double best_bid_d = strtod(best_bid, NULL);
  double best_ask_d = strtod(best_ask, NULL);

  uint32_t best_bid_fixed =
      (uint32_t)(best_bid_d * pow10[sec->display_precision]);
  uint32_t best_ask_fixed =
      (uint32_t)(best_ask_d * pow10[sec->display_precision]);

  sec->best_bid = best_bid_fixed;
  sec->best_ask = best_ask_fixed;
  sec->last_update = timestamp;

  chart_update(sec->chart, best_bid_fixed, best_ask_fixed, timestamp);

  return true;
}
