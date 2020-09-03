#include "chart.h"

struct chart *
chart_new(void)
{

  struct chart *cht = calloc(1, sizeof(struct chart));

  if (!cht) {
    pprint_error("%s@%s:%d unable to create chart no memory left (aborting)",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  }

  /*
   * This program works off of the weekly no more than the maximum number of
   * weekly candles will exist in memory at a time. At the end of the trading
   * week this buffer gets cleared and is put in cold storage on the disk
   */
  cht->candles = calloc(CHART_MINUTES_IN_WEEK, sizeof(struct candle));

  if (!(cht->candles)) {
    pprint_error("%s@%s:%d unable to create candle buffer no memory left "
                 "(aborting)",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  }

  /*
   * There are no objects availibale in an empty chart
   */
  cht->chart_objects = NULL;

  return cht;
}

void
chart_update(struct chart* cht, uint32_t bid, uint32_t ask, size_t timestamp)
{

  (void) cht;
  (void) bid;
  (void) ask;

  /*
   * The number of nanoseconds in a week
   */
  const size_t NANOSECONDS_IN_WEEK = 604800000000000;

  /*
   * The number of nanoseconds in a minute
   */
  const size_t NANOSECONDS_IN_MINUTE = 60000000000;

  printf("%lu\n", timestamp);

  size_t weeks_passed = timestamp / NANOSECONDS_IN_WEEK;
  size_t offset_since_beginning = timestamp - (weeks_passed * NANOSECONDS_IN_WEEK);
  size_t current_minute = offset_since_beginning / NANOSECONDS_IN_MINUTE;

  printf("%lu\n", current_minute);
  (void) NANOSECONDS_IN_MINUTE;

}
