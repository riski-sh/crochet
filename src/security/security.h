#ifndef SECURITY_H
#define SECURITY_H

#include <pprint/pprint.h>
#include <ffjson/ffjson.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef SECURITY_ANALYSIS_H
#include "analysis.h"
#endif

#ifndef CHART_H
#include "chart.h"
#endif

#include <string/string.h>

/*
 * A security object a security is also defined as "instrument" or "stock"
 */
struct security
{

  /*
   * The name of the security
   */
  char *name;

  /*
   * The pip location, for example
   *
   * 1.12345
   *      ^---- The pip location is at -4
   */
  int pip_location;

  /*
   * The display precision
   *
   * 1.12345
   *       ^--- We print up to 5 digits maximum
   */
  int display_precision;

  /*
   * The current best bid price
   */
  uint32_t best_bid;

  /*
   * The current best ask price
   */
  uint32_t best_ask;

  /*
   * The last update timestamp
   */
  uint64_t last_update;

  /*
   * A structure representing a chart, that defines this security
   */
  struct chart *chart;
};

/*
 * Creates a new security
 *
 * @param name the name of this security
 * @param pip_location the pip location
 * @param display_precision the display precision
 */
struct security *
security_new(char *name, int pip_location, int display_precision);

/*
 * Updates a security given a tick information
 */
bool
security_update(struct security *sec, uint64_t timestamp, char *best_bid,
                char *best_ask);
/*
 * Updates a security given the historical values
 *
 * @param sec the security to update
 * @param timestamp the timestamp of the start of the candle
 * @param o the opening of the candle
 * @param h the high of the candle
 * @param l the low of the candle
 * @param c the close of the candle
 */
bool
security_update_historical(struct security *sec, uint64_t timestamp, char *o,
                           char *h, char *l, char *c, uint32_t volume);

/*
 * Frees a security and sets it values to NULL
 *
 * @param sec the security to free
 */
void
security_free(struct security **sec);

/*
 * Generates the following json string
 * {
 *   "symbol": string,
 *   "bid": number,
 *   "ask": number,
 *   "precision": number
 * }
 *
 * @param sec the security to generate this data for
 * @param _data sets *_data to the allocated pointer with the json string
 * @param _len sets *_len to the value of the length of the string
 */
void
security_header_update(struct security *sec, char **_data, size_t *_len);

#endif
