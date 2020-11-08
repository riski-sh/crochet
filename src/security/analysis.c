#include "analysis.h"

void
chart_create_object_text(
    struct candle *cnd, char c, analysis_shortname_t shortname)
{
  /* create a generic object */
  struct chart_object *obj = NULL;
  obj = malloc(sizeof(struct chart_object) * 1);

  /* verify malloc allocted memory */
  if (!obj) {
    pprint_error(
        "unable to allocate %lu bytes (aborting)", sizeof(struct chart_object));
    exit(1);
  }

  /* set the shortname to understand who created this analysis */
  obj->shortname = shortname;

  /* we will add this analysis the beginning of the linked list */
  obj->next = cnd->analysis_list;

  /* define the object type to make sure readers cast the void* correctly */
  obj->object_type = CHART_OBJECT_TEXT;

  /* create a text object */
  struct chart_object_t_text *type = NULL;
  type = malloc(sizeof(struct chart_object_t_text) * 1);

  /* verify malloc allocated memory */
  if (!type) {
    pprint_error("unable to allocate %lu bytes (aborting)",
        sizeof(struct chart_object_t_text));
  }

  /* set the character to be displayed at the bottom of the candle */
  type->TEXT = c;

  /* point the generic object to the typed object */
  obj->value = (void *)type;

  /* make the newly created object the root of the list */
  cnd->analysis_list = obj;
}

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
