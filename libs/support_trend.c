#include <lib.h>
#include <math.h>
#include <stdbool.h>
#include "api.h"
#include "finmath/linear_equation.h"

static uint32_t
_candle_working_value(struct candle cnd)
{
  if (cnd.open > cnd.close) {
    return cnd.close;
  } else {
    return cnd.open;
  }
}

static bool
_valid_segment(struct candle *cnds, struct linear_equation *eq, int start, int end)
{
  if (start < 0) {
    return -1;
  }

  for (; start < end; ++start)
  {
    if (linear_equation_eval(eq, start) > _candle_working_value(cnds[start]))
    {
      return false;
    }
  }
  return true;

}

void
analysis_support_trend(struct candle *cnds, size_t indx)
{
  if (indx < 3) {
    return;
  }

  /*
   * At least three confirmations must occure so at max we can segment
   * the entire dataset into three sections
   */
  int maximum_segment = (indx - 1) / 3;

  /*
   * The last confirmed candle that will not change.
   */
  int starting_index = (indx - 1);
  int starting_price = _candle_working_value(cnds[starting_index]);

  while (maximum_segment > 1)
  {

    int last_confirmed = starting_index;
    int last_confirmed_price = _candle_working_value(cnds[last_confirmed]);

    /*
     * Begin by assuming a line between the last_confirmed candle and
     * last_confirmed - maximum_segment
     */
    struct linear_equation *eq = linear_equation_new(
        last_confirmed, _candle_working_value(cnds[last_confirmed]),
        last_confirmed - maximum_segment, _candle_working_value(cnds[last_confirmed - maximum_segment]));

    int confirmations = 1;

    while (_valid_segment(cnds, eq, last_confirmed - maximum_segment, last_confirmed))
    {
      bool last_confirmed_changed = false;
      for (int i = (last_confirmed - maximum_segment) - (maximum_segment / 2); i <= (last_confirmed - maximum_segment) + (maximum_segment / 2); ++i)
      {
        if (i >= 0) {
          if (cnds[i].low == linear_equation_eval(eq, i))
          {
            last_confirmed = i;
            last_confirmed_price = cnds[i].low;
            last_confirmed_changed = true;
          }
          else if (_candle_working_value(cnds[i]) == linear_equation_eval(eq, i))
          {
            last_confirmed = i;
            last_confirmed_price = _candle_working_value(cnds[i]);
            last_confirmed_changed = true;
          }
        }
      }
      if (!last_confirmed_changed)
      {
        break;
      }
      confirmations += 1;
    }

    if (confirmations >= 4)
    {
      (void) starting_price;
      (void) last_confirmed_price;
      linear_equation_free(&eq);
      chart_create_object_line(&(cnds[indx-1]), starting_index, starting_price,
          last_confirmed, last_confirmed_price, __func__);
      break;
    }

    linear_equation_free(&eq);
    maximum_segment -= 1;
  }

}

struct vtable exports = {
  &analysis_support_trend
};
