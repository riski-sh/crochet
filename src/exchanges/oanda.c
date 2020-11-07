#include "oanda.h"

static const char V3_ACCOUNTS_FMT[] = "/v3/accounts";
static const char V3_ACCOUNT_INSTRUMENTS[] = "/v3/accounts/%s/instruments";

static size_t
_oanda_timetots(char *str)
{
  double val = strtod(str, NULL);
  val *= 1000000000;
  return (size_t)val;
}

static char *
_oanda_first_id(char *response)
{
  __json_value _root = json_parse(response);
  __json_object _accounts = json_get_object(_root);
  __json_array _account = json_get_array(hashmap_get("accounts", _accounts));
  __json_object _id = json_get_object(_account->val);
  __json_string id = strdup(json_get_string(hashmap_get("id", _id)));

  json_free(_root);
  return id;
}

static void
_oanda_load_historical(struct http11request *request, struct security *sec)
{
  request->dirty = true;

  /* get the current timestamp */
  struct timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);
  size_t ts = time(NULL) * 1000000000;
  size_t backfill = chart_tstoidx(ts);

  if (backfill > 5000) {
    backfill = 5000;
  }

  char stub[120] = { '\x0' };
  sprintf(stub, "/v3/instruments/%s/candles?count=%lu&price=B&granularity=M1",
      sec->name, backfill);

  request->stub = stub;
  char *response = NULL;
  http11request_push(request, &response);

  __json_value _root = json_parse(response);
  __json_object _robj = json_get_object(_root);
  __json_array _candles = json_get_array(hashmap_get("candles", _robj));

  /*
   * loop through all historical candles and add them to the current
   * chart
   */
  while (_candles) {
    __json_object _candle = json_get_object(_candles->val);
    __json_string _timestamp = json_get_string(hashmap_get("time", _candle));
    __json_object _bids = json_get_object(hashmap_get("bid", _candle));

    __json_string o, h, l, c = NULL;
    __json_number volume;
    o = json_get_string(hashmap_get("o", _bids));
    h = json_get_string(hashmap_get("h", _bids));
    l = json_get_string(hashmap_get("l", _bids));
    c = json_get_string(hashmap_get("c", _bids));
    volume = json_get_number(hashmap_get("volume", _candle));

    size_t latest_timestamp = _oanda_timetots(_timestamp);
    if (!security_update_historical(sec, latest_timestamp, o, h, l, c, (uint32_t) *volume)) {
      pprint_error(
          "unable to push historical candle ts=%lu o=%s h=%s l=%s c=%s",
          latest_timestamp, o, h, l, c);
    }
    _candles = _candles->nxt;
  }

  json_free(_root);
  free(response);
}

static char *
_oanda_gen_currency_list(
    struct http11request *request, char *response, int *num_instruments)
{
  __json_value _root = json_parse(response);
  __json_object _instruments = json_get_object(_root);
  __json_array instruments =
      json_get_array(hashmap_get("instruments", _instruments));

  char *currency_list = malloc(1);
  currency_list[0] = '\x0';
  size_t total_len = 0;

  while (instruments) {
    __json_object instrument = json_get_object(instruments->val);
    char *name = json_get_string(hashmap_get("name", instrument));
    int pip_location =
        (int)*(json_get_number(hashmap_get("pipLocation", instrument)));
    int display_precision =
        (int)*(json_get_number(hashmap_get("displayPrecision", instrument)));

    struct security *sec = security_new(name, pip_location, display_precision);
    exchange_put(name, sec);

    /* load the past 24 hours worth of candles */
    _oanda_load_historical(request, sec);

    (*num_instruments) += 1;

    total_len += 8;
    currency_list = realloc(currency_list, total_len + 1);
    if (currency_list) {
      strcat(currency_list, name);
      strcat(currency_list, ",");
      currency_list[total_len] = '\x0';
    }

    instruments = instruments->nxt;
  }
  currency_list[total_len - 1] = '\x0';

  json_free(_root);
  return currency_list;
}

void *
exchanges_oanda_init(void *key)
{

  // create a reusable record for openssl read
  struct tls_session *master_session = NULL;
  if (tls_session_new(OANDA_API_ROOT, "443", &master_session) != STATUS_OK) {
    pprint_error("%s", "|");
    exit(1);
  }

  struct http11request *request = NULL;
  if (http11request_new(master_session, &request) != STATUS_OK) {
    pprint_error("%s", "|");
    exit(1);
  }

  request->stub = strdup(V3_ACCOUNTS_FMT);
  if (request->stub == NULL) {
    pprint_error("%s", "unable to duplicate stub");
    exit(1);
  }

  char *authtoken = malloc(strlen("Bearer ") + strlen(key) + 1);
  memcpy(authtoken, "Bearer \x0", 8);
  strcat(authtoken, key);

  hashmap_put("Authorization", authtoken, request->headers);
  hashmap_put("Accept-DateTime-Format", "UNIX", request->headers);
  hashmap_put("User-Agent", "crochet", request->headers);
  hashmap_put("Content-Type", "application/json", request->headers);
  hashmap_put("Accept", "*/*", request->headers);

  char *response = NULL;
  http11request_push(request, &response);
  free(request->stub);

  char *id = _oanda_first_id(response);

  free(response);
  response = NULL;

  pprint_info("using oanda account id %s", id);

  int get_instrument_size = 0;
  get_instrument_size = snprintf(NULL, 0, V3_ACCOUNT_INSTRUMENTS, id) + 1;

  char *get_instrument_str = malloc((size_t)get_instrument_size * sizeof(char));
  sprintf(get_instrument_str, "/v3/accounts/%s/instruments", id);
  request->stub = get_instrument_str;

  request->dirty = true;
  http11request_push(request, &response);
  free(get_instrument_str);

  int number_monitored = 0;
  char *instrument_update_end =
      _oanda_gen_currency_list(request, response, &number_monitored);

  pprint_info("oanda: loaded %d symbols", number_monitored);

  char *instrument_update_beg = "/v3/accounts/%s/pricing?instruments=";
  char *instrument_update_full = NULL;

  instrument_update_full = calloc(strlen(id) + strlen(instrument_update_beg) +
          strlen(instrument_update_end) + 2,
      sizeof(char));
  sprintf(instrument_update_full, "/v3/accounts/%s/pricing?instruments=%s", id,
      instrument_update_end);

  free(response);
  response = NULL;

  request->stub = instrument_update_full;
  request->dirty = true;

  int num_messages = 0;
  int num_valid_updates = 0;

  struct timespec start_time;
  struct timespec end_time;
  struct timespec result;

  __json_value _response_root = NULL;

  response = NULL;
  pprint_info("%s", "starting oanda main loop...");

  struct timespec speed_monitor_start;
  clock_gettime(CLOCK_REALTIME, &speed_monitor_start);
  struct timespec speed_monitor_end;

  struct timespec sleeper;

  while (globals_continue(NULL)) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);

    http11request_push(request, &response);

    if (_response_root == NULL) {
      _response_root = json_parse(response);
    } else {
      size_t idx = 0;
      json_parse_cached(response, &idx, _response_root);
    }
    __json_object _response_data = json_get_object(_response_root);
    __json_array _prices =
        json_get_array(hashmap_get("prices", _response_data));

    while (_prices) {
      __json_object _price_object = json_get_object(_prices->val);
      __json_string _price_instrument =
          json_get_string(hashmap_get("instrument", _price_object));
      __json_string price_update_time =
          json_get_string(hashmap_get("time", _price_object));

      size_t latest_timestamp = _oanda_timetots(price_update_time);

      __json_array _bids = json_get_array(hashmap_get("bids", _price_object));
      __json_array _asks = json_get_array(hashmap_get("asks", _price_object));

      __json_object _best_bid_bucket = json_get_object(_bids->val);
      __json_object _best_ask_bucket = json_get_object(_asks->val);

      __json_string best_bid =
          json_get_string(hashmap_get("price", _best_bid_bucket));
      __json_string best_ask =
          json_get_string(hashmap_get("price", _best_ask_bucket));

      struct security *working_sec = exchange_get(_price_instrument);

      if (security_update(working_sec, latest_timestamp, best_bid, best_ask)) {
        num_valid_updates += 1;
      }

      _prices = _prices->nxt;
    }

    num_messages += 1;

    clock_gettime(CLOCK_MONOTONIC_RAW, &speed_monitor_end);
    if (speed_monitor_end.tv_sec - speed_monitor_start.tv_sec >=
        (int) OANDA_PRINT_NTERVAL_SECONDS) {

      if (num_messages > 30) {
        pprint_error("oanda: %lu / 30 requests, please slow me down", num_messages);
      } else {
        pprint_info("oanda: %lu / 30 requests", num_messages);
      }

      clock_gettime(CLOCK_REALTIME, &speed_monitor_start);
      clock_gettime(CLOCK_REALTIME, &speed_monitor_end);
      num_messages = 0;
    }

    client_redraw();

    clock_gettime(CLOCK_MONOTONIC_COARSE, &end_time);
    result.tv_sec = end_time.tv_sec - start_time.tv_sec;
    result.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;

    size_t duration = ((result.tv_sec * 1000000000) + result.tv_nsec);

    int slowdown = (33333333 - duration);

    if (slowdown > 0) {
      sleeper.tv_nsec = slowdown;
      clock_nanosleep(CLOCK_MONOTONIC_RAW, 0, &sleeper, NULL);
    }

    printf("slowdown: %d\n", slowdown);
  }

  pprint_info("%s", "cleaning up exchange oanda...");

  free(response);
  json_free(_response_root);
  free(instrument_update_full);
  free(instrument_update_end);
  free(id);
  tls_session_free(&master_session);

  pprint_info("%s", "finished oanda cleanup");
  return NULL;
}
