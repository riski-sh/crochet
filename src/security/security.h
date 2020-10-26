#ifndef SECURITY_H
#define SECURITY_H

#include <ffjson/ffjson.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "chart.h"

/*
 * A security object a security is also defined as "instrument" or "stock"
 */
struct security {

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
  size_t last_update;

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
struct security *security_new(
    char *name, int pip_location, int display_precision);

/*
 * Updates a security given a tick information
 */
bool security_update(
    struct security *sec, size_t timestamp, char *best_bid, char *best_ask);

#endif
