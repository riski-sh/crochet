#include "comms.h"

static void
__parse_update_header(char **_response, size_t *_len)
{
  static char *(HEADER_SYMBOLS)[5] = {
    "EUR_USD",
    "USD_JPY",
    "GBP_USD",
    "AUD_USD",
    "USD_CAD",
  };

  static size_t NUM_HEADER_SYMBOLS = 5;

  struct string_t response;
  string_new(&response);

  string_append(&response,
      "{\"type\": \"update\", \"what\": \"header\", \"data\": [",
      strlen("{\"type\": \"update\", \"what\": \"header\", \"data\": ["));

  char *update_array_element = NULL;
  size_t element_length = 0;

  for (size_t i = 0; i < NUM_HEADER_SYMBOLS; ++i)
  {
    struct security *sec = exchange_get(HEADER_SYMBOLS[i]);

    if (!sec)
    {
      pprint_warn("requested header update for %s but %s is not a known "
                  "symbol ", HEADER_SYMBOLS[i]);
      continue;
    }
    security_header_update(sec, &update_array_element, &element_length);
    string_append(&response, update_array_element, element_length);

    if (i != NUM_HEADER_SYMBOLS - 1) {
      string_append(&response, ",", 1);
    }
    free(update_array_element);
  }
  string_append(&response, "]}", 2);

  *_response = response.data;
  *_len = response.len;
}

static void
__parse_update_chart_full(__json_object __root_obj, char **_response,
                          size_t *_len)
{

  __json_string __secu = json_get_string(hashmap_get("secu", __root_obj));

  struct security *sec = exchange_get(__secu);

  if (!sec)
  {
    pprint_warn("requested header update for %s but %s is not a known "
                  "symbol ", __secu);
    *_response = NULL;
    *_len = 0;
    return;
  }

  struct string_t response;
  string_new(&response);

  string_append(&response,
      "{\"type\": \"update\", \"what\": \"chart-full\", \"data\": [",
      strlen("{\"type\": \"update\", \"what\": \"chart-full\", \"data\": ["));

  struct chart *cht = sec->chart;

  /* send to the client the last 24 hours worth of candles */
  char *data = NULL;
  size_t len = 0;
  for (size_t i = 0; i < cht->cur_candle_idx; ++i)
  {
    struct candle *cnd = &(cht->candles[i]);
    chart_candle_json(cnd, i, &data, &len);
    string_append(&response, data, len);
    free(data);
    string_append(&response, ",", 1);
  }

  pthread_mutex_lock(&(cht->last_candle_grab));
  struct candle *cnd = &(cht->candles[cht->cur_candle_idx]);
  chart_candle_json(cnd, cht->cur_candle_idx, &data, &len);
  pthread_mutex_unlock(&(cht->last_candle_grab));

  string_append(&response, data, len);
  free(data);

  char buf[256];
  int ret = snprintf(buf, 255, "], \"precision\": %d}", sec->display_precision);

  if (ret == 256)
  {
    pprint_error("%s",
        "reached max length of buffer go back and make true dynamic TODO");
    exit(1);
  }

  string_append(&response, buf, (size_t) ret);

  *_response = response.data;
  *_len = response.len;
}

static void
__parse_update_chart_partial(__json_object __root_obj, char **_response,
                             size_t *_len)
{
  __json_string __secu = json_get_string(hashmap_get("secu", __root_obj));

  struct security *sec = exchange_get(__secu);

  if (!sec)
  {
    pprint_warn("requested header update for %s but %s is not a known "
                  "symbol ", __secu);
    *_response = NULL;
    *_len = 0;
    return;
  }

  struct string_t response;
  string_new(&response);

  string_append(&response,
      "{\"type\": \"update\", \"what\": \"chart-partial\", \"data\": [",
      strlen("{\"type\": \"update\", \"what\": \"chart-partial\", \"data\": ["));

  struct chart *cht = sec->chart;

  char *data = NULL;
  size_t len = 0;
  pthread_mutex_lock(&(cht->last_candle_grab));
  struct candle *cnd = &(cht->candles[cht->cur_candle_idx]);
  chart_candle_json(cnd, cht->cur_candle_idx, &data, &len);
  pthread_mutex_unlock(&(cht->last_candle_grab));

  string_append(&response, data, len);
  free(data);

  char buf[256];
  int ret = snprintf(buf, 255, "], \"bid\": %d, \"ask\": %d}",
      sec->best_bid, sec->best_ask);

  if (ret == 256)
  {
    pprint_error("%s",
        "reached max length of buffer go back and make true dynamic TODO");
    exit(1);
  }

  string_append(&response, buf, (size_t) ret);

  *_response = response.data;
  *_len = response.len;
}

static void
__parse_update_chart_last_valid(__json_object __root_obj, char **_response,
                                size_t *_len)
{

  __json_string __secu = json_get_string(hashmap_get("secu", __root_obj));

  struct security *sec = exchange_get(__secu);

  if (!sec)
  {
    pprint_warn("requested header update for %s but %s is not a known "
                  "symbol ", __secu);
    *_response = NULL;
    *_len = 0;
    return;
  }

  /* grab the last candle that won't change */
  struct candle *cnd = NULL;
  cnd = &(sec->chart->candles[sec->chart->cur_candle_idx - 1]);
  chart_candle_json(cnd, sec->chart->cur_candle_idx - 1, _response, _len);
}

static void
__parse_type_update(__json_object __root_obj, char **_response, size_t *_len)
{
  __json_string __what = json_get_string(hashmap_get("what", __root_obj));

  if (!__what)
  {
    pprint_error("%s", "malformed json request, property where can not be NULL");
    *_response = NULL;
    *_len = 0;
    return;
  }

  if (strcmp(__what, "header") == 0)
  {
    __parse_update_header(_response, _len);
  }
  else if (strcmp(__what, "chart-full") == 0)
  {
    __parse_update_chart_full(__root_obj, _response, _len);
  }
  else if (strcmp(__what, "chart-partial") == 0)
  {
    __parse_update_chart_partial(__root_obj, _response, _len);
  }
  else if (strcmp(__what, "chart-last-valid") == 0)
  {
    __parse_update_chart_last_valid(__root_obj, _response, _len);
  }
}

void
comms_parse_message(char *msg, char **_response, size_t *_len)
{
  __json_value __root = json_parse(msg);
  __json_object __root_obj = json_get_object(__root);
  __json_string __type = json_get_string(hashmap_get("type", __root_obj));

  if (!__type)
  {
    *_response = NULL;
    *_len = 0;

    pprint_error("%s", "malformed JSON request from client");
    json_free(__root);
    return;
  }

  if (strcmp(__type, "update") == 0)
  {
    __parse_type_update(__root_obj, _response, _len);
  }

  json_free(__root);
}
