#include "oanda.h"
#include <time.h>

/*
 * String formats for basic OANDA endpoints
 */

/* Queries basic account information */
static const char V3_ACCOUNTS_FMT[] = "/v3/accounts";

/*
 * Gets the tradeable instruments for the account which is the first
 * string format argument
 */
static const char V3_ACCOUNT_INSTRUMENTS[] = "/v3/accounts/%s/instruments";

/*
 * Converts the OANDA string represents the timestamp into a nanosecond
 * EPOCH tick.
 */
static uint64_t
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

  /* set the request to dirty to secure a non cached request */
  request->dirty = true;

  /* get the current timestamp */
  struct timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);

  uint64_t ts = (uint64_t) time(NULL) * 1000000000;

  size_t backfill = chart_tstoidx(ts);

  if (backfill == 0)
  {
    /*
     * don't update because there are no candles this week or we are in the
     * first minute
     */
    return;
  }

  if (backfill > 5000)
  {
    pprint_warn("%s only backfilling 5000 candles instead of %lu", sec->name,
                backfill);
    backfill = 5000;
  }

  /* generate the GET request url */
  char stub[120] = {'\x0'};
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
  while (_candles && globals_continue(NULL))
  {
    __json_object _candle = json_get_object(_candles->val);
    __json_string _timestamp = json_get_string(hashmap_get("time", _candle));
    __json_object _bids = json_get_object(hashmap_get("bid", _candle));

    __json_string o, h, l, c = NULL;
    __json_number volume;

    /* extract the open/high/low/close/volume of the candle in the list */
    o = json_get_string(hashmap_get("o", _bids));
    h = json_get_string(hashmap_get("h", _bids));
    l = json_get_string(hashmap_get("l", _bids));
    c = json_get_string(hashmap_get("c", _bids));
    volume = json_get_number(hashmap_get("volume", _candle));

    /* convert the string timestamp to a nanosecond EPOCH tick */
    uint64_t latest_timestamp = _oanda_timetots(_timestamp);

    /* update the chart */
    if (!security_update_historical(sec, latest_timestamp, o, h, l, c,
                                    (uint32_t)*volume))
    {
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
_oanda_gen_currency_list(struct http11request *request, char *response,
                         int *num_instruments)
{

  pprint_info("%s", "configuring tradeable oanda symbols...");
  __json_value _root = json_parse(response);
  __json_object _instruments = json_get_object(_root);
  __json_array instruments =
      json_get_array(hashmap_get("instruments", _instruments));

  char *currency_list = malloc(1);
  currency_list[0] = '\x0';
  size_t total_len = 0;

  while (instruments && globals_continue(NULL))
  {
    __json_object instrument = json_get_object(instruments->val);
    char *name = json_get_string(hashmap_get("name", instrument));
    int pip_location =
        (int)*(json_get_number(hashmap_get("pipLocation", instrument)));
    int display_precision =
        (int)*(json_get_number(hashmap_get("displayPrecision", instrument)));

    struct security *sec = security_new(name, pip_location, display_precision);
    exchange_put(name, sec);

    /* load at max the last 5000 minutes of historical data */
    pprint_info("loading historial info for %s...", name);
    _oanda_load_historical(request, sec);

    (*num_instruments) += 1;

    total_len += 8;
    currency_list = realloc(currency_list, total_len + 1);
    if (currency_list)
    {
      strcat(currency_list, name);
      strcat(currency_list, ",");
      currency_list[total_len] = '\x0';
    }

    instruments = instruments->nxt;
  }
  currency_list[total_len - 1] = '\x0';
  json_free(_root);

  pprint_info("%s", "finished loading historical symbols");

  return currency_list;
}

void *
exchanges_oanda_init(void *key)
{

  /* create a reusable record for openssl read */
  struct tls_session *master_session = NULL;
  if (tls_session_new(OANDA_API_ROOT, "443", &master_session) != STATUS_OK)
  {
    pprint_error("%s", "|");
    exit(1);
  }

  /* cconnect the master_sesssion to OANDA as a persistant connection */
  struct http11request *request = NULL;
  if (http11request_new(master_session, &request) != STATUS_OK)
  {
    pprint_error("%s", "|");
    exit(1);
  }

  /* setup basic headers that will be used in all requests to OANDA */

  /* api key */
  char *authtoken = malloc(strlen("Bearer ") + strlen(key) + 1);
  memcpy(authtoken, "Bearer \x0", 8);
  strcat(authtoken, key);
  hashmap_put("Authorization", authtoken, request->headers);

  /* request a unix timestamp and not a windows timestamp */
  hashmap_put("Accept-DateTime-Format", "UNIX", request->headers);

  /* tell OANDA that crochet is connecting */
  hashmap_put("User-Agent", "crochet", request->headers);

  /* only accept requests that are json responses */
  hashmap_put("Content-Type", "application/json", request->headers);

  /* accept any type response format */
  hashmap_put("Accept", "*/*", request->headers);

  /* keep this connection alive as much as possible */
  hashmap_put("Connection", "Keep-Alive", request->headers);

  /* the master response that will get passed to http11 client */
  char *response = NULL;

  /* duplicate the accounts format */
  request->stub = strdup(V3_ACCOUNTS_FMT);
  if (request->stub == NULL)
  {
    pprint_error("%s", "unable to duplicate stub");
    exit(1);
  }

  /* request account information */
  http11request_push(request, &response);
  free(request->stub);

  /* get the ID out of the response */
  char *id = _oanda_first_id(response);

  /* clean up this response as we will never request another account again */
  free(response);
  response = NULL;

  pprint_info("%s", "using oanda account id [REDACTED]");

  /* query all tradable currencies */
  int get_instrument_size = 0;
  get_instrument_size = snprintf(NULL, 0, V3_ACCOUNT_INSTRUMENTS, id) + 1;

  /* query to get all the securities */
  char *get_instrument_str = malloc((size_t)get_instrument_size * sizeof(char));
  sprintf(get_instrument_str, "/v3/accounts/%s/instruments", id);
  request->stub = get_instrument_str;

  request->dirty = true;
  http11request_push(request, &response);
  free(get_instrument_str);

  /* backfill the security data and create an instrument list */
  int number_monitored = 0;
  char *instrument_update_end =
      _oanda_gen_currency_list(request, response, &number_monitored);

  pprint_info("oanda: loaded %d symbols", number_monitored);

  /* setup polling request */
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

  /* counters for monitoring status */

  /* the number of messages in a second counter */
  double num_messages = 0;

  /*
   * the number of updates that have actually happened and not the number of
   * requests
   */
  int num_valid_updates = 0;

  /* timers */
  struct timespec start_time;
  struct timespec result;

  /* json that holds the tree for the response */
  __json_value _response_root = NULL;

  /* clear the response to force first allocation */
  response = NULL;
  pprint_info("%s", "starting oanda main loop...");

  /* setup clocks for the speed limiters */
  struct timespec speed_monitor_start;
  clock_gettime(CLOCK_MONOTONIC, &speed_monitor_start);
  struct timespec speed_monitor_end;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  /* continue iff there isn't a global kill signal */
  while (globals_continue(NULL))
  {

    /* perform the request */
    http11request_push(request, &response);

    /* full parse first time, cached parse everytime else */
    if (_response_root == NULL)
    {
      _response_root = json_parse(response);
    }
    else
    {
      size_t idx = 0;
      json_parse_cached(response, &idx, _response_root);
    }

    /* extract data from the response */
    __json_object _response_data = json_get_object(_response_root);
    __json_array _prices =
        json_get_array(hashmap_get("prices", _response_data));

    /* loop through each security */
    while (_prices)
    {
      __json_object _price_object = json_get_object(_prices->val);

      /* get the instrument */
      __json_string _price_instrument =
          json_get_string(hashmap_get("instrument", _price_object));

      /* get the time of the price update */
      __json_string price_update_time =
          json_get_string(hashmap_get("time", _price_object));

      uint64_t latest_timestamp = _oanda_timetots(price_update_time);

      /* get the bid order book and the ask order book */
      __json_array _bids = json_get_array(hashmap_get("bids", _price_object));
      __json_array _asks = json_get_array(hashmap_get("asks", _price_object));

      /* get the best bid and ask from the bucket */
      __json_object _best_bid_bucket = json_get_object(_bids->val);
      __json_object _best_ask_bucket = json_get_object(_asks->val);

      __json_string best_bid =
          json_get_string(hashmap_get("price", _best_bid_bucket));
      __json_string best_ask =
          json_get_string(hashmap_get("price", _best_ask_bucket));

      /* query the security given by the instrument name in the response
       */
      struct security *working_sec = exchange_get(_price_instrument);

      /* update the security and increment the valid updates for meta info
       */
      if (security_update(working_sec, latest_timestamp, best_bid, best_ask))
      {
        num_valid_updates += 1;
      }

      _prices = _prices->nxt;
    }

    num_messages += 1;

    /*
     * print out the number of requests that happened in the past second
     * OANDA specifies that no more than 30 requests a second per connection
     */
    clock_gettime(CLOCK_MONOTONIC, &speed_monitor_end);
    result.tv_sec = speed_monitor_end.tv_sec - speed_monitor_start.tv_sec;
    result.tv_nsec = speed_monitor_end.tv_nsec - speed_monitor_start.tv_nsec;
    size_t speed_duration =
        (((size_t)result.tv_sec * 1000000000L) + (size_t)result.tv_nsec);
    if (speed_duration >= (int)OANDA_PRINT_INTERVAL_SECONDS)
    {
      num_messages = (double)((num_messages / (double)speed_duration) *
                              OANDA_PRINT_INTERVAL_SECONDS);
      pprint_info("currently polling at %05.2f / %d r/s", num_messages, 30);
      clock_gettime(CLOCK_MONOTONIC, &speed_monitor_start);
      num_messages = 0;
    }

    /*
     * monitor the time that it took to perform this single request and
     * introduce and artificial sleep to make sure we don't get kicked out
     * for polling to fast
     */
    uint64_t start_nanoseconds =
        (uint64_t)start_time.tv_sec * (uint64_t)1000000000L +
        (uint64_t)start_time.tv_nsec;
    uint64_t end_nanoseconds = start_nanoseconds;
    do
    {
      struct timespec cur_time;
      clock_gettime(CLOCK_MONOTONIC, &cur_time);
      end_nanoseconds = (size_t)cur_time.tv_sec * (uint64_t)1000000000L +
                        (size_t)cur_time.tv_nsec;
    } while (end_nanoseconds - start_nanoseconds <= (uint64_t)3.3e7);

    clock_gettime(CLOCK_MONOTONIC, &start_time);
  }

  pprint_info("%s", "cleaning up exchange oanda...");

  free(response);
  if (_response_root)
  {
    json_free(_response_root);
  }
  http11request_free(&request);
  free(instrument_update_end);
  free(authtoken);
  free(id);

  pprint_info("%s", "finished oanda cleanup");
  return NULL;
}
