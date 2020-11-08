#ifndef CHART_H
#define CHART_H

#include <pprint/pprint.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SECURITY_ANALYSIS_H
#include "analysis.h"
#else
/*
 * Every type of chart object that can be drawn.
 */
typedef enum { CHART_OBJECT_TEXT = 0 } chart_object_t;

typedef enum {
  WHITE_MARUBUZU = 0,
  BLACK_MARUBUZU = 1,
  LONG_LEGGED_DRAGON_FLY_DOJI = 2,
  DRAGON_FLY_DOJI = 3,
  GRAVESTONE_DOJI = 4,
  FOUR_PRICE_DOJI = 5,
  HANGING_MAN = 6,
  SHOOTING_STAR = 7,
  SPINNING_TOP = 8
} analysis_shortname_t;

/*
 * Represents a generic object that gets displayed on the chart
 */
struct chart_object {
  chart_object_t object_type;
  analysis_shortname_t shortname;
  void *value;
  struct chart_object *next;
};

struct chart_object_t_text {
  char TEXT;
};

/*
 * Represents a candle in the chart
 */
struct candle {
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

#endif

#define CHART_MINUTES_IN_WEEK 10080
/*
 * Represents a chart and all the elements needed for analysis to draw
 * correctly and informative information on the chart
 */
struct chart {

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

  /*
   * A list of chart objects that were added to the chart
   */
  struct chart_object *chart_objects;
};

/*
 * Creates a new chart
 */
struct chart *chart_new(void);

/*
 * Updates the current chart
 *
 * @param cht the chart that will be updated
 * @param bid the best bid price
 * @param ask the best ask price
 */
void chart_update(
    struct chart *cht, uint32_t bid, uint32_t ask, size_t timestamp);

/*
 * Converts timestamp to offset where 0 is 0 minutes since sunday.
 */
size_t chart_tstoidx(size_t timestamp);

/*
 * Reset the entire chart
 */
void chart_reset(struct chart *cht);

#endif
