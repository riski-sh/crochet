#ifndef SECURITY_ANALYSIS_H
#define SECURITY_ANALYSIS_H

#ifndef CHART_H
#include "chart.h"
#else
/*
 * Every type of chart object that can be drawn.
 */
typedef enum { CHART_OBJECT_TEXT = 0 } chart_object_t;

/*
 * Represents a generic object that gets displayed on the chart
 */
struct chart_object {
  chart_object_t object_type;
  void *value;
  struct chart_object *next;
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

/*
 * Generic definition for analysis
 *
 * @param cnds A list of candles
 * @param indx The last candle + 1. Loops should be performed on the interval
 * [0, indx)
 */
typedef void (*analysis_func)(struct candle *cnds, size_t indx);

/*
 * Checks if candle is marubuzu (bullish/bearish).
 * Follows generic analysis function.
 */
void analysis_check_white_marubuzu(struct candle *cnds, size_t indx);
void analysis_check_black_marubuzu(struct candle *cnds, size_t indx);

/*
 * Checks if candle is doji
 */
void analysis_check_ll_dragonfly_doji(struct candle *cnds, size_t indx);
void analysis_check_dragonfly_doji(struct candle *cnds, size_t indx);
void analysis_check_gravestone_doji(struct candle *cnds, size_t indx);
void analysis_check_four_price_doji(struct candle *cnds, size_t indx);
void analysis_check_hanging_man(struct candle *cnds, size_t indx);
void analysis_check_shooting_star(struct candle *cnds, size_t indx);
void analysis_check_shooting_star(struct candle *cnds, size_t indx);
void analysis_check_spinning_top(struct candle *cnds, size_t indx);

#endif
