#include "httpws/http.h"
#include "oanda.h"

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

static char *
_oanda_gen_currency_list(char *response)
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

void
exchanges_oanda_init(char *key)
{
  struct httpwss_session *master_session =
      httpwss_session_new(OANDA_API_ROOT, "443");
  master_session->hashauth = true;
  master_session->authkey = key;

  char *response = NULL;
  http_get_request(master_session, "/v3/accounts", &response);

  char *id = _oanda_first_id(response);
  free(response);

  int get_instrument_size = 0;
  get_instrument_size =
      snprintf(NULL, 0, "/v3/accounts/%s/instruments", id) + 1;

  char *get_instrument_str = malloc((size_t)get_instrument_size * sizeof(char));
  sprintf(get_instrument_str, "/v3/accounts/%s/instruments", id);

  http_get_request(master_session, get_instrument_str, &response);
  free(get_instrument_str);

  char *instrument_update_end = _oanda_gen_currency_list(response);
  char *instrument_update_beg = "/v3/accounts/%s/pricing?instruments=";
  char *instrument_update_full = NULL;
  instrument_update_full = calloc(strlen(id) + strlen(instrument_update_beg) +
          strlen(instrument_update_end) + 2,
      sizeof(char));
  sprintf(instrument_update_full, "/v3/accounts/%s/pricing?instruments=EUR_USD", id);

  free(response);

  struct timespec start_time;

#if defined(__FreeBSD__)
  clock_gettime(CLOCK_UPTIME_PRECISE, &start_time);
#else
  clock_gettime(CLOCK_BOOTTIME, &start_time);
#endif

  struct timespec end_time;

  size_t num_msg = 0;

  while (globals_continue(NULL)) {
    http_get_request(master_session, instrument_update_full, &response);

    if (response == NULL) {
      pprint_info("oanda connection closed reconnecting...");
      httpwss_session_free(master_session);
      pprint_info("oanda connection reconnected");
      master_session = httpwss_session_new(OANDA_API_ROOT, "443");
      master_session->hashauth = true;
      master_session->authkey = key;
      pprint_info("oanda connection reconnected");
      continue;
    }

    __json_value _response_root = json_parse(response);
    json_free(_response_root);

    free(response);
#if defined(__FreeBSD__)
    clock_gettime(CLOCK_UPTIME_PRECISE, &end_time);
#else
    clock_gettime(CLOCK_BOOTTIME, &end_time);
#endif
    num_msg += 1;

    if ((end_time.tv_sec - start_time.tv_sec) >= 1) {
      double msg_ps =
          (double)num_msg / (double)(end_time.tv_sec - start_time.tv_sec);
      pprint_info("oanda feed message rate at %.2f msg/s", msg_ps);

#if defined(__FreeBSD__)
      clock_gettime(CLOCK_UPTIME_PRECISE, &end_time);
      clock_gettime(CLOCK_UPTIME_PRECISE, &start_time);
#else
      clock_gettime(CLOCK_BOOTTIME, &end_time);
      clock_gettime(CLOCK_BOOTTIME, &start_time);
#endif
      num_msg = 0;
    }
  }

  pprint_info("cleaning up exchange oanda...");
  free(instrument_update_full);
  free(instrument_update_end);
  free(id);
  httpwss_session_free(master_session);
}
