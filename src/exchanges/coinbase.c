#include "coinbase.h"
#include "ffjson/ffjson.h"
#include "httpws/wss.h"

static void
_clear_backlog(struct wss_session *coinbase_local, size_t snapshot_root)
{
  char *val = NULL;
  while (wss_read_text(coinbase_local, &val) == WSS_ERR_NONE) {
    __json_value msg = json_parse(val);
    __json_object o = json_get_object(msg);

    __json_value _sequence_local = hashmap_get("sequence", o);
    if (_sequence_local == NULL) {
      free(msg);
    } else {
      size_t sequence_local = (size_t)(*(json_get_number(_sequence_local)));
      if (sequence_local <= snapshot_root) {
        pprint_info("ignoring message (%lu<=%lu)", __FILE_NAME__, __func__,
            __LINE__, sequence_local, snapshot_root);
        json_free(msg);
        free(val);
        continue;
      } else {
        free(val);
        break;
      }
    }
  }
}

static void
_subscribe(char *id, struct wss_session *coinbase_local)
{
  char *garbage = NULL;
  int feed_request_len = snprintf(NULL, 0,
      "{\"type\": \"subscribe\",\"product_ids\": [\"%s\"],\"channels\": [\"full\"]}",
      (char *)id);
  char *full_request = NULL;
  full_request = malloc((size_t)feed_request_len + 1);
  sprintf(full_request,
      "{\"type\": \"subscribe\",\"product_ids\": [\"%s\"],\"channels\": [\"full\"]}",
      (char *)id);
  wss_send_text(
      coinbase_local, (unsigned char *)full_request, (size_t)feed_request_len);
  wss_read_text(coinbase_local, &garbage);
  free(garbage);
}

static coinbase_book *
_build_book(__json_array side)
{

  coinbase_book *root = NULL;
  while (side) {

    __json_array elements = json_get_array(side->val);

    char *_price = NULL;
    char *_size = NULL;
    char *order_id = NULL;
    for (size_t i = 0; i < 3; ++i) {
      switch (i) {
      case 0:
        _price = json_get_string(elements->val);
        break;
      case 1:
        _size = json_get_string(elements->val);
        break;
      case 2:
        order_id = strdup(json_get_string(elements->val));
        break;
      }
      elements = elements->nxt;
    }

    uint64_t price_cents = usdtocent_str(_price);
    uint64_t size_satoshi = btctosat_str(_size);

    struct coinbase_value *e = malloc(sizeof(struct coinbase_value));
    e->nxt = NULL;
    e->orderid = order_id;
    e->size = size_satoshi;

    coinbase_book_put(&root, price_cents, e);

    side = side->nxt;
  }

  return root;
}

/*
 * Pulls the full order book and starts processing the order book values.
 *
 * The order of operations is as so.
 *
 * 1. A fork will be created. The child process keeps a message queue these
 *    messages do not get parsed until the parent process has finished.
 *
 * 2. In the parent process request the full order book. The order book is
 *    then processed and the sequence id of the order book is stored.
 *
 * 3. Once the full order book is processed the child process will exit.
 *
 * 4. After the child process exits the message back log gets processed and
 *    discards and messages that were received with a sequence number before
 *    the full order book pull.
 *
 * 5. After this, the parent process regains control of the socket connection
 *    and continues to process messages as they come in.
 *
 * @param id id is really a char* and contains the id needed to pull the
 * coinbase full orderbook.
 */
static void
_coinbase_start(void *id)
{
  pprint_info(
      "starting exchange %s", __FILE_NAME__, __func__, __LINE__, (char *)id);

  pprint_info("storing full order book for %s", __FILE_NAME__, __func__,
      __LINE__, (char *)id);

  // Connect to the websocket feed
  struct wss_session coinbase_local;
  if (wss_client("ws-feed.pro.coinbase.com", "/", "443", &coinbase_local) !=
      WSS_ERR_NONE) {
    pprint_error("unable to connect to coinbase websocket feed", __FILE_NAME__,
        __func__, __LINE__);
    abort();
  }

  // Subscribe to the feed
  _subscribe((char *)id, &coinbase_local);

  char *full_book_request = NULL;
  int full_book_request_len =
      snprintf(NULL, 0, "/products/%s/book?level=3", (char *)id);

  full_book_request = malloc((size_t)full_book_request_len + 1);
  sprintf(full_book_request, "/products/%s/book?level=3", (char *)id);

  coinbase_book *bid_book = NULL;
  coinbase_book *ask_book = NULL;

  char *buffer = NULL;
  http_get_request(COINBASE_API, full_book_request, &buffer);

  free(full_book_request);
  __json_value full_book = json_parse(buffer);
  __json_object book = json_get_object(full_book);

  __json_value _sequence = hashmap_get("sequence", book);
  __json_array bids = json_get_array(hashmap_get("bids", book));
  __json_array asks = json_get_array(hashmap_get("asks", book));
  size_t snapshot_root = (size_t)(*(json_get_number(_sequence)));

  pprint_info("storing full order book for %s", __FILE_NAME__, __func__,
      __LINE__, (char *)id);
  bid_book = _build_book(bids);
  ask_book = _build_book(asks);
  pprint_info("finished full order book for %s", __FILE_NAME__, __func__,
      __LINE__, (char *)id);

  json_free(full_book);

  pprint_info("syncing to realtime for %s", __FILE_NAME__, __func__, __LINE__,
      (char *)id);
  _clear_backlog(&coinbase_local, snapshot_root);
  pprint_info("syncing to realtime for %s", __FILE_NAME__, __func__, __LINE__,
      (char *)id);

  pprint_info("started normal message flow for %s", __FILE_NAME__, __func__,
      __LINE__, (char *)id);

  char *msg_rt = NULL;
  while (wss_read_text(&coinbase_local, &msg_rt) == WSS_ERR_NONE) {
    free(msg_rt);
  }
}

void
coinbase_init()
{
  char *full_products = NULL;

  if (http_get_request(COINBASE_API, "/products", &full_products) != 0) {
    pprint_error("unable to get %s%s", __FILE_NAME__, __func__, __LINE__,
        COINBASE_API, "/products");
    abort();
  }

  __json_value prod_list_json = json_parse(full_products);
  __json_array prods_json = json_get_array(prod_list_json);
  while (prods_json) {
    __json_value product = prods_json->val;
    __json_object product_data = product->data;

    __json_value _quote_currency = hashmap_get("quote_currency", product_data);
    char *quote_currency = json_get_string(_quote_currency);

    __json_value _base_currency = hashmap_get("base_currency", product_data);
    char *base_currency = json_get_string(_base_currency);

    if (strcmp(quote_currency, "USD") != 0 ||
        strcmp(base_currency, "BTC") != 0) {
      prods_json = prods_json->nxt;
      continue;
    }

    __json_value _id = hashmap_get("id", product_data);
    char *id = json_get_string(_id);

    _coinbase_start(id);

    prods_json = prods_json->nxt;
  }

  json_free(prod_list_json);
  free(full_products);
}
