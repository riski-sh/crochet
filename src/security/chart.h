#ifndef CHART_H
#define CHART_H

#include <api.h>
#include <pprint/pprint.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string/string.h>

#include "analysis.h"

#define CHART_MINUTES_IN_WEEK 10080

/*
 * Creates a new chart
 */
struct chart *
chart_new(void);

/*
 * Updates the current chart
 *
 * @param cht the chart that will be updated
 * @param bid the best bid price
 * @param ask the best ask price
 */
void
chart_update(struct chart *cht, uint32_t bid, uint32_t ask, size_t timestamp);

/*
 * Converts timestamp to offset where 0 is 0 minutes since sunday.
 */
size_t
chart_tstoidx(size_t timestamp);

/*
 * Reset the entire chart. This is used during the weekly reset of the chart.
 *
 * @param cht the chart to reset
 */
void
chart_reset(struct chart *cht);

/*
 * Runs analysis on this chart
 *
 * @param cht the chart to perform analysis on
 * @param cndidx the last working candle + 1
 */
void
chart_runanalysis(struct chart *cht, size_t cndidx);

/*
 * Frees the chart and sets the value to NULL
 *
 * @param cht the chart to free
 */
void
chart_free(struct chart **cht);

/*
 * Given a candle, seralizes the data into a json object
 *
 * @param cnd the candle to serialize
 * @param _data will allocate *_data, caller owns data and must free
 * @param _len the length of the data
 */
void
chart_candle_json(struct candle *cnd, size_t index, char **_data, size_t *_len);

#endif
