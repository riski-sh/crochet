#include "security.h"
#include "pprint/pprint.h"
#include "security/chart.h"
#include <stdio.h>

struct security *
security_new(char *name, int pip_location, int display_precision)
{
  struct security *sec = calloc(1, sizeof(struct security));
  sec->name = strdup(name);
  sec->pip_location = pip_location;
  sec->display_precision = display_precision;
  sec->log_path_tick = NULL;
  sec->last_update = 0;
  sec->chart = chart_new();

  return sec;
}

bool
security_update(struct security *sec, uint64_t timestamp, char *best_bid,
                char *best_ask)
{
  static const int pow10[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};

  if (timestamp == sec->last_update)
  {
    return false;
  }

  chart_timestamp_log_path(timestamp, "OANDA", sec->name,
      &(sec->log_path_tick), &(sec->current_day));

  FILE *fp = fopen(sec->log_path_tick, "a+");

  if (!fp)
  {
    pprint_warn("unable to log to path %s", sec->log_path_tick);
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

  if (fp)
  {
    fprintf(fp, "%d,%d,%lu\n", best_bid_fixed, best_ask_fixed, timestamp);
  }

  chart_update(sec->chart, best_bid_fixed, best_ask_fixed, timestamp);

  fflush(fp);
  fclose(fp);
  return true;
}

bool
security_update_historical(struct security *sec, uint64_t timestamp, char *o,
                           char *h, char *l, char *c, uint32_t volume)
{
  static const int pow10[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};

  double open, high, low, close;
  open = strtod(o, NULL);
  high = strtod(h, NULL);
  low = strtod(l, NULL);
  close = strtod(c, NULL);

  uint32_t open_fixed, high_fixed, low_fixed, close_fixed;
  open_fixed = (uint32_t)(open * pow10[sec->display_precision]);
  high_fixed = (uint32_t)(high * pow10[sec->display_precision]);
  low_fixed = (uint32_t)(low * pow10[sec->display_precision]);
  close_fixed = (uint32_t)(close * pow10[sec->display_precision]);

  size_t cndidx = chart_tstoidx(timestamp);

  if (cndidx > CHART_MINUTES_IN_WEEK)
  {
    pprint_error("malformed data in historical security impossible index %lu",
                 cndidx);
    return false;
  }

  sec->chart->candles[cndidx].open = open_fixed;
  sec->chart->candles[cndidx].high = high_fixed;
  sec->chart->candles[cndidx].low = low_fixed;
  sec->chart->candles[cndidx].close = close_fixed;
  sec->chart->candles[cndidx].volume = volume;

  sec->chart->cur_candle_idx = cndidx;
  sec->best_bid = close_fixed;
  sec->best_ask = close_fixed;
  sec->last_update = timestamp;

  chart_runanalysis(sec->chart, cndidx + 1);

  return true;
}

void
security_header_update(struct security *sec, char **_data, size_t *_len)
{
  int json_len = snprintf(NULL, 0,
      "{\"symbol\": \"%s\", \"bid\": %d, \"ask\": %d, \"precision\": %d}",
      sec->name, sec->best_bid, sec->best_ask, sec->display_precision);

  char *update_str = calloc((size_t)(json_len + 1), sizeof(char));

  PPRINT_CHECK_ALLOC(update_str);

  int wrote = snprintf(update_str, (size_t) json_len + 1,
      "{\"symbol\": \"%s\", \"bid\": %d, \"ask\": %d, \"precision\": %d}",
      sec->name, sec->best_bid, sec->best_ask, sec->display_precision);

  if (wrote != json_len)
  {
    pprint_error("%s", "did not write the expected amount for header_update");
    exit(1);
  }

  *_data = update_str;
  *_len = (size_t) json_len;
}

void
security_free(struct security **sec)
{
  free((*sec)->name);
  (*sec)->name = NULL;

  chart_free(&(*sec)->chart);

  free(*sec);
  (*sec) = NULL;
}
