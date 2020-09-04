#include "chart.h"

typedef enum {
  THURSDAY = 0,
  FRIDAY = 1,
  SATURDAY = 2,
  SUNDAY = 3,
  MONDAY = 4,
  TUESDAY = 5,
  WEDNESDAY = 6,

  // number of days in a week
  NUM_DOW_T = 7
} dow_t;

/*
 * Convert dow_t to numbers of days since previous sunday
 */
static size_t days_since_sunday[NUM_DOW_T] = { 4, 5, 6, 0, 1, 2, 3 };

/*
 * the number of nanoseconds in a minute
 */
static const size_t NANOSECONDS_IN_MINUTE = 60000000000;

/*
 * the number of nanoseconds in a day
 */
static const size_t NANOSECONDS_IN_DAY = 8.64e+13;

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

/*
 * Converts a nanosecond timestamp to the day of week
 */
static dow_t
_tstodow(size_t timestamp)
{
  size_t ts_sec = timestamp / 1000000000;
  size_t days_since_epoch = ts_sec / 86400;
  return (dow_t)days_since_epoch % 7;
}

void
chart_update(struct chart *cht, uint32_t bid, uint32_t ask, size_t timestamp)
{

  (void)cht;
  (void)bid;
  (void)ask;

  /*
   * Get the current day.
   */
  dow_t current_day = _tstodow(timestamp);

  /*
   * Convert out day into days since sunday
   */
  size_t positive_sunday_offset = days_since_sunday[current_day];

  /*
   * Compute the number of minutes that have passed since sunday
   *
   * (CUR_TS - SUN_TS) / NANOSECONDS_IN_MINUTES
   *
   */
  size_t beginning_of_day = timestamp - (timestamp % NANOSECONDS_IN_DAY);
  size_t beginning_of_week =
      beginning_of_day - (positive_sunday_offset * NANOSECONDS_IN_DAY);
  size_t minutes_since_sunday =
      (timestamp - beginning_of_week) / NANOSECONDS_IN_MINUTE;

  (void) minutes_since_sunday;

}
