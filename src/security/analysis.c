#include "analysis.h"
#include "security/chart.h"

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
      pprint_info("%s", "found spinning top");
      struct chart_object *obj = NULL;
      obj = malloc(sizeof(struct chart_object) * 1);
      obj->shortname = SPINNING_TOP;

      if (!obj) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object));
      }

      obj->next = cnds[indx - 1].analysis_list;
      obj->object_type = CHART_OBJECT_TEXT;

      struct chart_object_t_text *type = NULL;

      type = malloc(sizeof(struct chart_object_t_text) * 1);

      if (!type) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object_t_text));
      }

      type->TEXT = 'T';

      obj->value = (void *)type;

      cnds[indx - 1].analysis_list = obj;
    }
  } else if (cnds[indx - 1].close < cnds[indx - 1].open) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    x = cnds[indx - 1].high - cnds[indx - 1].open;
    y = cnds[indx - 1].open - cnds[indx - 1].close;
    z = cnds[indx - 1].close - cnds[indx - 1].low;

    uint32_t s = (x > z) ? x - z : z - x;

    if (x > 0 && z > 0 && y > 0 && x + z > y && s <= 2) {
      pprint_info("%s", "found spinning top");
      struct chart_object *obj = NULL;
      obj = malloc(sizeof(struct chart_object) * 1);
      obj->shortname = SPINNING_TOP;

      if (!obj) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object));
      }

      obj->next = cnds[indx - 1].analysis_list;
      obj->object_type = CHART_OBJECT_TEXT;

      struct chart_object_t_text *type = NULL;

      type = malloc(sizeof(struct chart_object_t_text) * 1);

      if (!type) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object_t_text));
      }

      type->TEXT = 'T';

      obj->value = (void *)type;

      cnds[indx - 1].analysis_list = obj;
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
      struct chart_object *obj = NULL;
      obj = malloc(sizeof(struct chart_object) * 1);
      obj->shortname = SHOOTING_STAR;

      if (!obj) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object));
      }

      obj->next = cnds[indx - 1].analysis_list;
      obj->object_type = CHART_OBJECT_TEXT;

      struct chart_object_t_text *type = NULL;

      type = malloc(sizeof(struct chart_object_t_text) * 1);

      if (!type) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object_t_text));
      }

      type->TEXT = 'S';

      obj->value = (void *)type;

      cnds[indx - 1].analysis_list = obj;
    }
  } else if (cnds[indx - 1].close < cnds[indx - 1].open) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    z = cnds[indx - 1].high - cnds[indx - 1].open;
    y = cnds[indx - 1].open - cnds[indx - 1].close;
    x = cnds[indx - 1].close - cnds[indx - 1].low;

    if (x < z && z > 2 * x && y < z) {
      struct chart_object *obj = NULL;
      obj = malloc(sizeof(struct chart_object) * 1);
      obj->shortname = SHOOTING_STAR;

      if (!obj) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object));
      }

      obj->next = cnds[indx - 1].analysis_list;
      obj->object_type = CHART_OBJECT_TEXT;

      struct chart_object_t_text *type = NULL;

      type = malloc(sizeof(struct chart_object_t_text) * 1);

      if (!type) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object_t_text));
      }

      type->TEXT = 'S';

      obj->value = (void *)type;

      cnds[indx - 1].analysis_list = obj;
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
      struct chart_object *obj = NULL;
      obj = malloc(sizeof(struct chart_object) * 1);
      obj->shortname = HANGING_MAN;

      if (!obj) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object));
      }

      obj->next = cnds[indx - 1].analysis_list;
      obj->object_type = CHART_OBJECT_TEXT;

      struct chart_object_t_text *type = NULL;

      type = malloc(sizeof(struct chart_object_t_text) * 1);

      if (!type) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object_t_text));
      }

      type->TEXT = 'H';

      obj->value = (void *)type;

      cnds[indx - 1].analysis_list = obj;
    }
  } else if (cnds[indx - 1].close < cnds[indx - 1].open) {
    /* calculate x,y,z for candles who was bullish */

    /* close > open therefore high - close > 0 */
    x = cnds[indx - 1].high - cnds[indx - 1].open;
    y = cnds[indx - 1].open - cnds[indx - 1].close;
    z = cnds[indx - 1].close - cnds[indx - 1].low;

    if (x < z && z > 2 * x && y < z) {
      struct chart_object *obj = NULL;
      obj = malloc(sizeof(struct chart_object) * 1);
      obj->shortname = HANGING_MAN;

      if (!obj) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object));
      }

      obj->next = cnds[indx - 1].analysis_list;
      obj->object_type = CHART_OBJECT_TEXT;

      struct chart_object_t_text *type = NULL;

      type = malloc(sizeof(struct chart_object_t_text) * 1);

      if (!type) {
        pprint_error("unable to allocate %lu bytes (aborting)",
            sizeof(struct chart_object_t_text));
      }

      type->TEXT = 'H';

      obj->value = (void *)type;

      cnds[indx - 1].analysis_list = obj;
    }
  }
}

void
analysis_check_four_price_doji(struct candle *cnds, size_t indx)
{

  if (cnds[indx - 1].open == cnds[indx - 1].close &&
      cnds[indx - 1].close == cnds[indx - 1].low &&
      cnds[indx - 1].close == cnds[indx - 1].high) {

    struct chart_object *obj = NULL;
    obj = malloc(sizeof(struct chart_object) * 1);
    obj->shortname = FOUR_PRICE_DOJI;

    if (!obj) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object));
    }

    obj->next = cnds[indx - 1].analysis_list;
    obj->object_type = CHART_OBJECT_TEXT;

    struct chart_object_t_text *type = NULL;

    type = malloc(sizeof(struct chart_object_t_text) * 1);

    if (!type) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object_t_text));
    }

    type->TEXT = 'D';

    obj->value = (void *)type;

    cnds[indx - 1].analysis_list = obj;
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

    struct chart_object *obj = NULL;
    obj = malloc(sizeof(struct chart_object) * 1);
    obj->shortname = GRAVESTONE_DOJI;

    if (!obj) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object));
    }

    obj->next = cnds[indx - 1].analysis_list;
    obj->object_type = CHART_OBJECT_TEXT;

    struct chart_object_t_text *type = NULL;

    type = malloc(sizeof(struct chart_object_t_text) * 1);

    if (!type) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object_t_text));
    }

    type->TEXT = 'D';

    obj->value = (void *)type;

    cnds[indx - 1].analysis_list = obj;
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
    struct chart_object *obj = NULL;
    obj = malloc(sizeof(struct chart_object) * 1);
    obj->shortname = DRAGON_FLY_DOJI;

    if (!obj) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object));
    }

    obj->next = cnds[indx - 1].analysis_list;
    obj->object_type = CHART_OBJECT_TEXT;

    struct chart_object_t_text *type = NULL;

    type = malloc(sizeof(struct chart_object_t_text) * 1);

    if (!type) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object_t_text));
    }

    type->TEXT = 'D';

    obj->value = (void *)type;

    cnds[indx - 1].analysis_list = obj;
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

    struct chart_object *obj = NULL;
    obj = malloc(sizeof(struct chart_object) * 1);
    obj->shortname = LONG_LEGGED_DRAGON_FLY_DOJI;

    if (!obj) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object));
    }

    obj->next = cnds[indx - 1].analysis_list;
    obj->object_type = CHART_OBJECT_TEXT;

    struct chart_object_t_text *type = NULL;

    type = malloc(sizeof(struct chart_object_t_text) * 1);

    if (!type) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object_t_text));
    }

    type->TEXT = 'D';

    obj->value = (void *)type;

    cnds[indx - 1].analysis_list = obj;
  }
}

void
analysis_check_black_marubuzu(struct candle *cnds, size_t indx)
{
  if (cnds[indx - 1].open == cnds[indx - 1].high &&
      cnds[indx - 1].close == cnds[indx - 1].low &&
      cnds[indx - 1].open != cnds[indx - 1].close) {
    struct chart_object *black_marubuzu = NULL;
    black_marubuzu = malloc(sizeof(struct chart_object) * 1);
    black_marubuzu->shortname = BLACK_MARUBUZU;

    if (!black_marubuzu) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object));
    }

    black_marubuzu->next = cnds[indx - 1].analysis_list;
    black_marubuzu->object_type = CHART_OBJECT_TEXT;

    struct chart_object_t_text *type = NULL;

    type = malloc(sizeof(struct chart_object_t_text) * 1);

    if (!type) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object_t_text));
    }

    type->TEXT = 'M';

    black_marubuzu->value = (void *)type;

    cnds[indx - 1].analysis_list = black_marubuzu;
  }
}
void
analysis_check_white_marubuzu(struct candle *cnds, size_t indx)
{
  if (cnds[indx - 1].open == cnds[indx - 1].low &&
      cnds[indx - 1].close == cnds[indx - 1].high &&
      cnds[indx - 1].open != cnds[indx - 1].close) {
    struct chart_object *white_marubuzu = NULL;
    white_marubuzu = malloc(sizeof(struct chart_object) * 1);
    white_marubuzu->shortname = WHITE_MARUBUZU;

    if (!white_marubuzu) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object));
    }

    white_marubuzu->next = cnds[indx - 1].analysis_list;
    white_marubuzu->object_type = CHART_OBJECT_TEXT;

    struct chart_object_t_text *type = NULL;

    type = malloc(sizeof(struct chart_object_t_text) * 1);

    if (!type) {
      pprint_error("unable to allocate %lu bytes (aborting)",
          sizeof(struct chart_object_t_text));
    }

    type->TEXT = 'M';

    white_marubuzu->value = (void *)type;

    cnds[indx - 1].analysis_list = white_marubuzu;
  }
}
