#ifndef SECURITY_ANALYSIS_H
#define SECURITY_ANALYSIS_H

#ifndef CHART_H
#include "chart.h"
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
