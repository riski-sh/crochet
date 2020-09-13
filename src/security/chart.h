#ifndef CHART_H
#define CHART_H

#include <pprint/pprint.h>
#include <stdint.h>
#include <stdlib.h>

#define CHART_MINUTES_IN_WEEK 10080

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
};

/*
 * Every type of chart object that can be drawn.
 */
typedef enum { CHART_REGION = 0, CHART_LINE = 0 } chart_object_t;

/*
 * Represents a generic object that gets displayed on the chart
 */
struct chart_object {
  chart_object_t object_type;

  // voluntary padding to align value
  char _p1[4];

  void *value;
};

/*
 * A shaded region on the chart. The shaded region on the chart will be drawn
 * over the encompassing candles. This region has opacity to allow for the
 * candles to still be seen.
 */
struct chart_region {
  /*
   * The starting candle index of the box
   */
  size_t startidx;

  /*
   * The ending candle index of the box
   */
  size_t endidx;
};

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

#endif
