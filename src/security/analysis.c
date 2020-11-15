#include "analysis.h"

static struct vtable **tables = NULL;
static size_t num_vtables = 0;

void
analysis_check_spinning_top(struct candle *cnds, size_t indx)
{
  uint32_t x, y, z;

  if (cnds[indx - 1].open < cnds[indx - 1].close) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    x = cnds[indx - 1].high - cnds[indx - 1].close;
    y = cnds[indx - 1].close - cnds[indx - 1].open;
    z = cnds[indx - 1].open - cnds[indx - 1].low;

    uint32_t s = (x > z) ? x - z : z - x;

    if (x > 0 && z > 0 && y > 0 && x + z > y && s <= 2) {
      chart_create_object_text(&(cnds[indx - 1]), 'T', SPINNING_TOP);
    }
  } else if (cnds[indx - 1].close < cnds[indx - 1].open) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    x = cnds[indx - 1].high - cnds[indx - 1].open;
    y = cnds[indx - 1].open - cnds[indx - 1].close;
    z = cnds[indx - 1].close - cnds[indx - 1].low;

    uint32_t s = (x > z) ? x - z : z - x;

    if (x > 0 && z > 0 && y > 0 && x + z > y && s <= 2) {
      chart_create_object_text(&(cnds[indx - 1]), 'T', SPINNING_TOP);
    }
  }
}

void
analysis_check_shooting_star(struct candle *cnds, size_t indx)
{
  uint32_t x, y, z;

  if (cnds[indx - 1].open < cnds[indx - 1].close) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    z = cnds[indx - 1].high - cnds[indx - 1].close;
    y = cnds[indx - 1].close - cnds[indx - 1].open;
    x = cnds[indx - 1].open - cnds[indx - 1].low;

    if (x < z && z > 2 * x && y < z) {
      chart_create_object_text(&(cnds[indx - 1]), 'S', SHOOTING_STAR);
    }
  } else if (cnds[indx - 1].close < cnds[indx - 1].open) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    z = cnds[indx - 1].high - cnds[indx - 1].open;
    y = cnds[indx - 1].open - cnds[indx - 1].close;
    x = cnds[indx - 1].close - cnds[indx - 1].low;

    if (x < z && z > 2 * x && y < z) {
      chart_create_object_text(&(cnds[indx - 1]), 'S', SHOOTING_STAR);
    }
  }
}

void
analysis_check_hanging_man(struct candle *cnds, size_t indx)
{
  uint32_t x, y, z;

  if (cnds[indx - 1].open < cnds[indx - 1].close) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    x = cnds[indx - 1].high - cnds[indx - 1].close;
    y = cnds[indx - 1].close - cnds[indx - 1].open;
    z = cnds[indx - 1].open - cnds[indx - 1].low;

    if (x < z && z > 2 * x && y < z) {
      chart_create_object_text(&(cnds[indx - 1]), 'H', SHOOTING_STAR);
    }
  } else if (cnds[indx - 1].close < cnds[indx - 1].open) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    x = cnds[indx - 1].high - cnds[indx - 1].open;
    y = cnds[indx - 1].open - cnds[indx - 1].close;
    z = cnds[indx - 1].close - cnds[indx - 1].low;

    if (x < z && z > 2 * x && y < z) {
      chart_create_object_text(&(cnds[indx - 1]), 'H', SHOOTING_STAR);
    }
  }
}

void
analysis_check_four_price_doji(struct candle *cnds, size_t indx)
{

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].close == cnds[indx - 1].low &&
      cnds[indx - 1].close == cnds[indx - 1].high) {
    chart_create_object_text(&(cnds[indx - 1]), 'D', FOUR_PRICE_DOJI);
  }
}

void
analysis_check_gravestone_doji(struct candle *cnds, size_t indx)
{

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].open == cnds[indx - 1].high &&
      cnds[indx - 1].open == cnds[indx - 1].low) {
    return;
  }

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].close == cnds[indx - 1].low) {

    chart_create_object_text(&(cnds[indx - 1]), 'D', GRAVESTONE_DOJI);
  }
}

void
analysis_check_dragonfly_doji(struct candle *cnds, size_t indx)
{

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].open == cnds[indx - 1].high &&
      cnds[indx - 1].open == cnds[indx - 1].low) {
    return;
  }

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].close == cnds[indx - 1].high) {

    chart_create_object_text(&(cnds[indx - 1]), 'D', DRAGON_FLY_DOJI);
  }
}

void
analysis_check_ll_dragonfly_doji(struct candle *cnds, size_t indx)
{

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].open == cnds[indx - 1].high &&
      cnds[indx - 1].open == cnds[indx - 1].low) {
    return;
  }

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].close != cnds[indx - 1].high &&
      cnds[indx - 1].close != cnds[indx - 1].low) {

    chart_create_object_text(
        &(cnds[indx - 1]), 'D', LONG_LEGGED_DRAGON_FLY_DOJI);
  }
}

void
analysis_check_black_marubuzu(struct candle *cnds, size_t indx)
{
  if (cnds[indx - 1].open == cnds[indx - 1].high &&
      cnds[indx - 1].close == cnds[indx - 1].low &&
      cnds[indx - 1].open != cnds[indx - 1].close) {

    chart_create_object_text(&(cnds[indx - 1]), 'M', BLACK_MARUBUZU);
  }
}

void
analysis_check_white_marubuzu(struct candle *cnds, size_t indx)
{
  if (cnds[indx - 1].open == cnds[indx - 1].low &&
      cnds[indx - 1].close == cnds[indx - 1].high &&
      cnds[indx - 1].open != cnds[indx - 1].close) {

    chart_create_object_text(&(cnds[indx - 1]), 'M', WHITE_MARUBUZU);
  }
}

void
analysis_resistance_line(struct candle *cnds, size_t indx)
{
  /* don't continue unless we have at least 4 candles */
  if (indx < 3) {
    return;
  }

  /* only find a support on a candle that is not a doji */
  if (cnds[indx - 1].open == cnds[indx - 1].close) {
    return;
  }

  size_t start_idx = indx - 1;
  size_t start_price = 0;

  size_t end_idx = 0;
  size_t end_price = 0;

  if (cnds[indx - 1].open > cnds[indx - 1].close) {
    start_price = cnds[indx - 1].open;
  } else {
    start_price = cnds[indx - 1].close;
  }

  size_t confirmations = 1;
  for (size_t idx = indx - 2; idx > 0; idx -= 1) {
    if (cnds[idx].close > start_price) {
      break;
    }
    if (cnds[idx].close == start_price) {
      end_idx = idx;
      end_price = cnds[idx].close;
      confirmations += 1;
      continue;
    }
    if (cnds[idx].high == start_price) {
      end_idx = idx;
      end_price = cnds[idx].high;
      confirmations += 1;
      continue;
    }
    break;
  }

  if (end_price != 0 && confirmations >= 3) {
    chart_create_object_line(&(cnds[start_idx]), start_idx, start_price,
        end_idx, end_price, RESISTANCE_LINE);
  }
}

void
analysis_run(struct candle* cnds, size_t indx)
{
  for (size_t i = 0; i < num_vtables; ++i) {
    tables[i]->run(cnds, indx);
  }
}

void
analysis_init()
{
  dlopen (NULL, RTLD_NOW|RTLD_GLOBAL);
  void *handle = dlopen("./libs/marubuzu.so", RTLD_LAZY);
  if (!handle) {
    pprint_error("%s", "unable to load ./libs/marubuzu.so (aborting)");
    pprint_error("dlopen error: %s", dlerror());
    exit(1);
  }
  pprint_info("%s", "loading libs/marubuzu.so...");

  struct vtable *libclass = dlsym(handle, "exports");
  if (!libclass) {
    pprint_error("%s", "this library does not export a vtable (aborting)");
    exit(1);
  }

  num_vtables += 1;
  tables = realloc(tables, sizeof(struct vtable*) * num_vtables);
  tables[num_vtables - 1] = libclass;

  pprint_info("%s", "loading libs/marubuzu.so... vtable OK");
}
