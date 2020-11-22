#include <lib.h>

static void
analysis_check_white_marubuzu(struct candle *cnds, size_t indx)
{
  if (cnds[indx - 1].open == cnds[indx - 1].low &&
      cnds[indx - 1].close == cnds[indx - 1].high &&
      cnds[indx - 1].open != cnds[indx - 1].close)
  {

    chart_create_object_text(&(cnds[indx - 1]), 'M', __func__);
  }
}

struct vtable exports = {&analysis_check_white_marubuzu};
