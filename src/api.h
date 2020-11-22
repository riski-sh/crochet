#ifndef CROCHET_API
#define CROCHET_API

#include <pprint/pprint.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Every type of chart object that can be drawn.
 */
typedef enum
{
  CHART_OBJECT_TEXT = 0,
  CHART_OBJECT_LINE = 1
} chart_object_t;

/*
 * Represents a generic object that gets displayed on the chart
 */
struct chart_object
{
  chart_object_t object_type;
  const char *name;
  void *value;
  struct chart_object *next;
};

struct chart_object_t_text
{
  char TEXT;
};

struct chart_object_t_line
{
  size_t start;
  uint32_t start_price;

  size_t end;
  uint32_t end_price;
};

/*
 * Represents a candle in the chart
 */
struct candle
{
  /*
   *    |   <---- high
   *    |
   *  ----- <---- open or close
   *  |   |
   *  |   |
   *  |   |
   *  |   |
   *  ----- <---- close or open
   *    |
   *    |   <---- low
   *
   */
  uint32_t open;
  uint32_t high;
  uint32_t low;
  uint32_t close;
  uint32_t volume;

  struct chart_object *analysis_list;
};

#define CHART_MINUTES_IN_WEEK 10080
/*
 * Represents a chart and all the elements needed for analysis to draw
 * correctly and informative information on the chart
 */
struct chart
{

  /*
   * Number of candles used in the buffer
   */
  size_t num_candles;

  /*
   * The candle index of the current candle
   */
  size_t cur_candle_idx;

  /*
   * A list of candles by default the 1 minute
   */
  struct candle *candles;
};

/*
 * Generic definition for analysis
 *
 * @param cnds A list of candles
 * @param indx The last candle + 1. Loops should be performed on the interval
 * [0, indx)
 */
typedef void (*analysis_func)(struct candle *cnds, size_t indx);

/*
 * Generates a chart line
 *
 * @param cnd the candle to start drawing this line at
 * @param start_idx the start index of the candle
 * @param start_price the price to start drawing at
 * @param end_idx the ending candle index
 * @param end_price the ending price for the line
 * @param function_name the function name that called this function given by
 * the __func__ macro
 */
void
chart_create_object_line(struct candle *cnd, size_t start_idx,
                         uint32_t start_price, size_t end_idx,
                         uint32_t end_price, const char *function_name);

/*
 * Generates a character to draw under the chart
 *
 * @param cnd the candle to draw the text under
 * @param c the character to draw
 * @param c the function name that called this function given by the __func__
 * macro.
 */
void
chart_create_object_text(struct candle *cnd, char c, const char *function_name);

#endif
