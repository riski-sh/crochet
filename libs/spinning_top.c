#include <lib.h>

static void
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
      chart_create_object_text(&(cnds[indx - 1]), 'T', __func__);
    }
  } else if (cnds[indx - 1].close < cnds[indx - 1].open) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    x = cnds[indx - 1].high - cnds[indx - 1].open;
    y = cnds[indx - 1].open - cnds[indx - 1].close;
    z = cnds[indx - 1].close - cnds[indx - 1].low;

    uint32_t s = (x > z) ? x - z : z - x;

    if (x > 0 && z > 0 && y > 0 && x + z > y && s <= 2) {
      chart_create_object_text(&(cnds[indx - 1]), 'T', __func__);
    }
  }
}

struct vtable exports = {
  &analysis_check_spinning_top
};
