#include <lib.h>

static void
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

    chart_create_object_text(&(cnds[indx - 1]), 'D', __func__);
  }
}


struct vtable exports = {
  &analysis_check_ll_dragonfly_doji
};
