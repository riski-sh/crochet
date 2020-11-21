#include <lib.h>

void
analysis_check_four_price_doji(struct candle *cnds, size_t indx)
{

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].close == cnds[indx - 1].low &&
      cnds[indx - 1].close == cnds[indx - 1].high) {
    chart_create_object_text(&(cnds[indx - 1]), 'D', __func__);
  }
}

struct vtable exports = {
  &analysis_check_four_price_doji
};
