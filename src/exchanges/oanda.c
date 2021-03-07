#include "oanda.h"

/*
 * String formats for basic OANDA endpoints
 */

/* Queries basic account information */
static const char V3_ACCOUNTS_FMT[] = OANDA_API_ROOT "/v3/accounts";

/*
 * Gets the tradeable instruments for the account which is the first
 * string format argument
 */
static const char V3_ACCOUNT_INSTRUMENTS[] = OANDA_API_ROOT "/v3/accounts/%s/instruments";

static size_t
write_func(void *ptr, size_t size, size_t nmemb, struct string_t *s)
{

  size_t new_len = s->len + size*nmemb;
  s->data = realloc(s->data, new_len+1);
  if (s->data == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->data+s->len, ptr, size*nmemb);
  s->data[new_len] = '\0';
  s->len = new_len;
  return size*nmemb;
}

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
_oanda_load_historical(CURL *curl, struct security *sec)
{

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
  char url[120] = {'\x0'};
  sprintf(url, OANDA_API_ROOT "/v3/instruments/%s/candles?count=%lu&price=B&granularity=M1",
          sec->name, backfill);

  pprint_info("DEBUG: %s", url);

  struct string_t response;
  response.data = NULL;
  response.len = 0;

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_perform(curl);

  __json_value _root = json_parse(response.data);
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
  free(response.data);
  response.len = 0;
  response.data = NULL;
}

static char *
_oanda_gen_currency_list(CURL *curl, struct string_t *response,
                         int *num_instruments)
{

  pprint_info("%s", "configuring tradeable oanda symbols...");
  __json_value _root = json_parse(response->data);
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
    _oanda_load_historical(curl, sec);

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

  /* create a new curl handle */
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    pprint_error("%s", "unable to init curl aborting...");
  }

  /* construct api key header value */
  char *authtoken = malloc(strlen("Authorization: Bearer \n") + strlen(key) + 1);
  sprintf(authtoken, "Authorization: Bearer %s", key);

  /* build headers */
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, authtoken);
  headers = curl_slist_append(headers, "Accept-DateTime-Format: UNIX");
  headers = curl_slist_append(headers, "User-Agent: riski/crochet");
  headers = curl_slist_append(headers, "Accept: */*");

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


  struct string_t response;

  /* setup curl write function */
  curl_easy_setopt(curl, CURLOPT_URL, V3_ACCOUNTS_FMT);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  /* query json endpoing to account data */
  /* the master response that libcurl will write to */
  response.data = NULL;
  response.len = 0;
  curl_easy_perform(curl);

  /* get the ID out of the response */
  char *id = _oanda_first_id(response.data);

  pprint_info("%s", "using oanda account id [REDACTED]");

  /* query all tradable currencies */
  int get_instrument_size = 0;
  get_instrument_size = snprintf(NULL, 0, V3_ACCOUNT_INSTRUMENTS, id) + 1;

  /* query to get all the securities */
  char *get_instrument_str = malloc((size_t)get_instrument_size * sizeof(char));
  sprintf(get_instrument_str, V3_ACCOUNT_INSTRUMENTS, id);

  curl_easy_setopt(curl, CURLOPT_URL, get_instrument_str);

  free(response.data);
  response.data = NULL;
  response.len = 0;
  curl_easy_perform(curl);

  /* backfill the security data and create an instrument list */
  int number_monitored = 0;
  char *instrument_update_end =
      _oanda_gen_currency_list(curl, &response, &number_monitored);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  pprint_info("oanda: loaded %d symbols", number_monitored);

  /* setup polling request */
  char *instrument_update_beg = OANDA_API_ROOT "/v3/accounts/%s/pricing?instruments=";
  char *instrument_update_full = NULL;

  instrument_update_full = calloc(strlen(id) + strlen(instrument_update_beg) +
                                      strlen(instrument_update_end) + 2,
                                  sizeof(char));
  sprintf(instrument_update_full, OANDA_API_ROOT "/v3/accounts/%s/pricing?instruments=%s", id,
          instrument_update_end);

  pprint_info("DEBUG: %s", instrument_update_full);
  curl_easy_setopt(curl, CURLOPT_URL, instrument_update_full);

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
    free(response.data);
    response.data = NULL;
    response.len = 0;

    curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

    /* make sure we got a correct resposne */
    if (http_code != 200)
    {
      pprint_info("error recieved %s", response.data);
    }
    else
    {
      /* full parse first time, cached parse everytime else */
      if (_response_root == NULL)
      {
        _response_root = json_parse(response.data);
      }
      else
      {
        size_t idx = 0;
        json_parse_cached(response.data, &idx, _response_root);
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
    }

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


  free(response.data);
  response.data = NULL;
  response.len = 0;

  if (_response_root)
  {
    json_free(_response_root);
  }

  curl_easy_cleanup(curl);
  free(instrument_update_end);
  free(authtoken);
  free(id);

  pprint_info("%s", "finished oanda cleanup");
  return NULL;
}
