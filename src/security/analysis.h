#ifndef SECURITY_ANALYSIS_H
#define SECURITY_ANALYSIS_H

#include <api.h>
#include <lib.h>
#include <pprint/pprint.h>
#include <stdbool.h>
#include <dlfcn.h>


void analysis_init();
void analysis_run(struct candle* cnds, size_t indx);


/*
 * Checks if candle is marubuzu (bullish/bearish).
 * Follows generic analysis function.
 */
void analysis_check_white_marubuzu(struct candle *cnds, size_t indx);
void analysis_check_black_marubuzu(struct candle *cnds, size_t indx);

/*
 * Checks if candle is a type doji
 */
void analysis_check_ll_dragonfly_doji(struct candle *cnds, size_t indx);
void analysis_check_dragonfly_doji(struct candle *cnds, size_t indx);
void analysis_check_gravestone_doji(struct candle *cnds, size_t indx);
void analysis_check_four_price_doji(struct candle *cnds, size_t indx);
void analysis_check_hanging_man(struct candle *cnds, size_t indx);

/*
 * Checks for shooting stars
 */
void analysis_check_shooting_star(struct candle *cnds, size_t indx);
void analysis_check_shooting_star(struct candle *cnds, size_t indx);

/*
 * Checks for spinning tops
 */
void analysis_check_spinning_top(struct candle *cnds, size_t indx);

/*
 * Find trend lines
 */
void analysis_support_line(struct candle *cnds, size_t indx);
void analysis_resistance_line(struct candle *cnds, size_t indx);

#endif
