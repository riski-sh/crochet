#include "chart.h"
#include "api.h"
#include "security/analysis.h"

/*
 * Represents a day of week from the start of the epoch
 */
typedef enum
{
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
 * Updates a candle on the chart
 *
 * @param cht the chart to update
 * @param bid the bid price
 * @param idx the index of the candle
 */
static void
_chart_update_candle(struct chart *cht, uint32_t bid, size_t idx)
{
  struct candle *cnd = &(cht->candles[idx]);

  pthread_mutex_lock(&(cht->last_candle_grab));
  if (cnd->volume != 0)
  {
    if (bid > cnd->high)
    {
      cnd->high = bid;
    }
    if (bid < cnd->low)
    {
      cnd->low = bid;
    }
    cnd->close = bid;
    cnd->volume += 1;
  }
  else
  {
    cnd->close = bid;
    cnd->open = bid;
    cnd->high = bid;
    cnd->low = bid;

    cnd->volume += 1;
  }
  pthread_mutex_unlock(&(cht->last_candle_grab));

}

/*
 * Convert dow_t to numbers of days since previous sunday
 */
static size_t days_since_sunday[NUM_DOW_T] = {4, 5, 6, 0, 1, 2, 3};

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

  if (!cht)
  {
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

  if (!(cht->candles))
  {
    pprint_error("%s@%s:%d unable to create candle buffer no memory left "
                 "(aborting)",
                 __FILE_NAME__, __func__, __LINE__);
    abort();
  }

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

struct analysis_meta
{
  struct chart *cht;
  size_t cndidx;
};

void
chart_runanalysis(struct chart *cht, size_t cndidx)
{
  if (cndidx > 1)
  {
    analysis_run(cht->candles, cndidx);
  }
}

void
chart_update(struct chart *cht, uint32_t bid, uint32_t ask, size_t timestamp)
{

  /*
   * Unsure about what to do with the ask value
   * will keep in for now. This only keeps track of the bid price.
   */
  (void)ask;

  size_t minutes_since_sunday = chart_tstoidx(timestamp);

  if (minutes_since_sunday > cht->cur_candle_idx)
  {
    chart_runanalysis(cht, minutes_since_sunday);
  }

  if (minutes_since_sunday < cht->cur_candle_idx)
  {
    pprint_info("%s", "resetting chart");
    chart_reset(cht);
  }

  cht->cur_candle_idx = minutes_since_sunday;
  _chart_update_candle(cht, bid, minutes_since_sunday);
}

size_t
chart_tstoidx(size_t timestamp)
{
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

  // market opens at 5PM sunday EST time
  return minutes_since_sunday - 1320;
}

void
chart_reset(struct chart *cht)
{
  cht->cur_candle_idx = 0;
  memset(cht->candles, 0, sizeof(struct candle) * cht->num_candles);
  // TODO reset chart objects
}

void
chart_free(struct chart **cht)
{

  for (size_t i = 0; i < CHART_MINUTES_IN_WEEK; ++i)
  {
    struct chart_object *ll = (*cht)->candles[i].analysis_list;
    while (ll)
    {
      free(ll->value);
      struct chart_object *next = ll->next;
      free(ll);
      ll = next;
    }
    (*cht)->candles[i].analysis_list = NULL;
  }
  free((*cht)->candles);

  free(*cht);
  *cht = NULL;
}

void
chart_candle_json(struct candle *cnd, size_t index, char **_data, size_t *_len)
{
  struct string_t candle;
  string_new(&candle);

  int json_len = snprintf(NULL, 0,
      "{\"open\": %d, \"high\": %d, \"low\": %d, \"close\": %d, \"volume\": %d,"
      "\"index\": %lu, \"analysis\": [",
      cnd->open, cnd->high, cnd->low, cnd->close, cnd->volume, index);

  char *update_str = calloc((size_t)(json_len + 1), sizeof(char));

  PPRINT_CHECK_ALLOC(update_str);

  int wrote = snprintf(update_str, (size_t) json_len + 1,
      "{\"open\": %d, \"high\": %d, \"low\": %d, \"close\": %d, \"volume\": %d,"
      "\"index\": %lu, \"analysis\": [",
      cnd->open, cnd->high, cnd->low, cnd->close, cnd->volume, index);


  if (wrote != json_len)
  {
    pprint_error("%s", "did not write the expected amount for header_update");
    exit(1);
  }

  string_append(&candle, update_str, (size_t) json_len);
  free(update_str);
  update_str = NULL;

  /* loop through each chart object associated with the candle */
  struct chart_object *iter = cnd->analysis_list;

  while (iter)
  {
    switch (iter->object_type)
    {
      case CHART_OBJECT_LINE:
      {
        struct chart_object_t_line *line = iter->value;
        json_len = snprintf(NULL, 0,
            "{\"name\": \"%s\", \"type\": \"CHART_OBJECT_LINE\", \"end\": %lu, \"endPrice\": %d, \"start\": %lu, \"startPrice\": %d}",
            iter->name, line->end, line->end_price, line->start, line->start_price);
        update_str = calloc((size_t) (json_len + 1), sizeof(char));

        PPRINT_CHECK_ALLOC(update_str);

        wrote = snprintf(update_str, (size_t) json_len + 1,
            "{\"name\": \"%s\", \"type\": \"CHART_OBJECT_LINE\", \"end\": %lu, \"endPrice\": %d, \"start\": %lu, \"startPrice\": %d}",
            iter->name, line->end, line->end_price, line->start, line->start_price);

        if (wrote != json_len)
        {
          pprint_error("%s", "did not write the expected amount for CHART_OBJECT_LINE");
          exit(1);
        }
        string_append(&candle, update_str, (size_t) json_len);
        free(update_str);

        if (iter->next)
        {
          string_append(&candle, ",", 1);
        }

        break;
      }
      case CHART_OBJECT_TEXT:
      {
        struct chart_object_t_text *line = iter->value;
        json_len = snprintf(NULL, 0,
            "{\"name\": \"%s\", \"type\": \"CHART_OBJECT_TEXT\", \"char\": \"%c\"}",
            iter->name, line->TEXT);
        update_str = calloc((size_t) (json_len + 1), sizeof(char));

        PPRINT_CHECK_ALLOC(update_str);

        wrote = snprintf(update_str, (size_t) json_len + 1,
            "{\"name\": \"%s\", \"type\": \"CHART_OBJECT_TEXT\", \"char\": \"%c\"}",
            iter->name, line->TEXT);

        if (wrote != json_len)
        {
          pprint_error("%s", "did not write the expected amount for CHART_OBJECT_LINE");
          exit(1);
        }
        string_append(&candle, update_str, (size_t) json_len);
        free(update_str);

        if (iter->next)
        {
          string_append(&candle, ",", 1);
        }

        break;
      }
    }
    iter = iter->next;
  }

  string_append(&candle, "]}", 2);

  *_data = candle.data;
  *_len = (size_t) candle.len;
}
