#include "chart.h"
#include "api.h"
#include "security/analysis.h"
#include <pthread.h>

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


static void _mkdir(const char *dir) {
        char tmp[PATH_MAX];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}

void
chart_timestamp_log_path(uint64_t timestamp, char *exchange_name,
                         char *security_name, char **_tick_location,
                         uint64_t *_cache_day)
{
  /*
   * Get the number of nanoseconds in a day.
   */
  const uint64_t day_ns = 86400000000000;

  /*
   * The beginning of the day, eg. Tuesday Decement 12 2020 00:00:00 +0000
   */
  uint64_t beg_of_day = timestamp - (timestamp % day_ns);

  if (beg_of_day == *_cache_day)
  {
    return;
  }

  /*
   * A place to store the path
   */
  char path[PATH_MAX] = {0};
  sprintf(path, "./archive/%s/%lu/%s", exchange_name, beg_of_day, security_name);

  if (*_tick_location)
  {
    free(*_tick_location);
  }

  _mkdir(path);

  char tick_path[PATH_MAX] = {0};
  sprintf(tick_path, "%s/tick.csv", path);
  *_tick_location = strdup(tick_path);
  *_cache_day = beg_of_day;
}

size_t
chart_tstoidx(uint64_t timestamp)
{
  /*
   * Get the number of nanoseconds in a day.
   */
  const uint64_t day_ns = 86400000000000;

  /*
   * Number of nanoseconds in a minute
   */
  const uint64_t minute_ns = 60000000000;

  /*
   * Get the current day as an index. Where 0 is sunday
   */
  uint64_t day_of_week = (((timestamp / day_ns) % 7) + 4) % 7;

  /*
   * The beginning of the day, eg. Tuesday Decement 12 2020 00:00:00 +0000
   */
  uint64_t beg_of_day = timestamp - (timestamp % day_ns);

  /*
   * Nanoseconds since last sunday
   */
  uint64_t last_sunday_offset = day_of_week * day_ns;

  /*
   * The epoch time representing last sunday
   */
  uint64_t sunday_epoch = beg_of_day - last_sunday_offset;

  /*
   * The number of nanoseconds that have passed since last sunday
   */
  uint64_t nanoseconds_since_sunday = timestamp - sunday_epoch;

  /*
   * The number of minutes that have passed since last sunday
   */
  uint64_t minutes_since_sunday = nanoseconds_since_sunday / minute_ns;

  if (minutes_since_sunday < 1320)
  {
    minutes_since_sunday = 0;
  }
  else
  {
    minutes_since_sunday -= 1320;
  }

  return minutes_since_sunday;
}

void
chart_reset(struct chart *cht)
{
  cht->cur_candle_idx = 0;
  for (size_t i = 0; i < CHART_MINUTES_IN_WEEK; ++i)
  {
    struct chart_object *ll = cht->candles[i].analysis_list;
    while (ll)
    {
      free(ll->value);
      struct chart_object *next = ll->next;
      free(ll);
      ll = next;
    }
    cht->candles[i].analysis_list = NULL;
  }
  memset(cht->candles, 0, sizeof(struct candle) * cht->num_candles);
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
