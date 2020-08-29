#include "coinbase.h"

static void
_subscribe(char *id, struct httpwss_session *coinbase_local)
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
    e->prv = NULL;
    e->orderid = order_id;
    e->size = size_satoshi;
    e->open = true;

    coinbase_book_received(&root, price_cents, e);

    side = side->nxt;
  }

  return root;
}

static void
_parse_open(__json_object msg, coinbase_book **bid, coinbase_book **ask)
{
  // There is no open messages for market orders.
  // There also must be a received message before an open message is
  // found. Knowing this it is safe to assume that this is already on the book.
  // We must extract the UUID for this order, the book side it lives on,
  // the level and the remaining size.

  __json_string order_id = json_get_string(hashmap_get("order_id", msg));
  uint64_t price_cents =
      usdtocent_str(json_get_string(hashmap_get("price", msg)));
  uint64_t btc_sat_left =
      btctosat_str(json_get_string(hashmap_get("remaining_size", msg)));
  __json_string _side = json_get_string(hashmap_get("side", msg));

  if (strcmp(_side, "buy") == 0) {
    coinbase_book_open(bid, price_cents, btc_sat_left, order_id);
  } else if (strcmp(_side, "sell") == 0) {
    coinbase_book_open(ask, price_cents, btc_sat_left, order_id);
  } else {
    pprint_error(
        "%s@%s:%d unknown side (aborting)", __FILE_NAME__, __func__, __LINE__);
    abort();
  }
}

static void
_parse_done(__json_object msg, coinbase_book **bid, coinbase_book **ask)
{
  // A done message for a market order could happen but their json will not
  // have a "remaining_size" attribute. So check if that exists before
  // continuing.

  __json_value _size = hashmap_get("remaining_size", msg);
  __json_value _price = hashmap_get("price", msg);

  if (!_size || !_price) {
    pprint_warn("skipping market done");
    return;
  }

  // since we got here this isn't a market done message
  __json_string _side = json_get_string(hashmap_get("side", msg));
  __json_string uuid = json_get_string(hashmap_get("order_id", msg));

  uint64_t price_cents = usdtocent_str(json_get_string(_price));

  if (strcmp(_side, "buy") == 0) {
    coinbase_book_remove(bid, price_cents, uuid);
  } else if (strcmp(_side, "sell") == 0) {
    coinbase_book_remove(ask, price_cents, uuid);
  } else {
    pprint_error(
        "%s@%s:%d unknown side (aborting)", __FILE_NAME__, __func__, __LINE__);
    abort();
  }
}

static void
_parse_received(__json_object msg, coinbase_book **bid, coinbase_book **ask)
{
  __json_string _type = json_get_string(hashmap_get("order_type", msg));

  if (strcmp(_type, "market") == 0) {
    // TODO process the market order
    // for now it is safe to ignore market orders since they are special
    // and never put on the order book
    pprint_warn("skipping market order", __FILE_NAME__, __func__, __LINE__);
    return;
  } else if (strcmp(_type, "limit") == 0) {
    struct coinbase_value *v = malloc(sizeof(struct coinbase_value));
    v->nxt = NULL;
    v->prv = NULL;
    v->open = false;

    // Get the order id
    __json_value _orderid = hashmap_get("order_id", msg);
    char *orderid = strdup(json_get_string(_orderid));
    v->orderid = orderid;

    __json_string _price = json_get_string(hashmap_get("price", msg));
    uint64_t price_cents = usdtocent_str(_price);

    __json_string _btc = json_get_string(hashmap_get("size", msg));
    uint64_t btc_sat = btctosat_str(_btc);
    v->size = btc_sat;

    __json_string _side = json_get_string(hashmap_get("side", msg));

    if (strcmp(_side, "buy") == 0) {
      coinbase_book_received(bid, price_cents, v);
    } else if (strcmp(_side, "sell") == 0) {
      coinbase_book_received(ask, price_cents, v);
    } else {
      pprint_error("%s@%s:%d unknown side (aborting)", __FILE_NAME__, __func__,
          __LINE__);
      abort();
    }

  } else {
    pprint_error("%s@%s:%d unknown received message type (aborting)",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  }
}

static void
_parse_match(__json_object msg, coinbase_book **bid, coinbase_book **ask)
{
  __json_string _side = json_get_string(hashmap_get("side", msg));
  uint64_t price = usdtocent_str(json_get_string(hashmap_get("price", msg)));
  uint64_t size = btctosat_str(json_get_string(hashmap_get("size", msg)));
  __json_string maker_id = json_get_string(hashmap_get("maker_order_id", msg));

  if (strcmp(_side, "buy") == 0) {
    coinbase_book_match(bid, price, size, maker_id);
  } else if (strcmp(_side, "sell") == 0) {
    coinbase_book_match(ask, price, size, maker_id);
  } else {
    pprint_error(
        "%s@%s:%d unknown side (aborting)", __FILE_NAME__, __func__, __LINE__);
    abort();
  }
}

/*
 * Pulls the full order book and starts processing the order book values.
 *
 * The order of operations is as so.
 *
 * 1. Start a web socket connection and let it start streaming data to us.
 * 2. Pull the order book after starting the web socket connection.
 * 3. Read the buffered messages and maintain the full book
 *
 * @param id id is really a char* and contains the id needed to pull the
 * coinbase full orderbook.
 */
static void
_coinbase_start(void *id)
{
  pprint_info("starting exchange %s", (char *)id);

  pprint_info("storing full order book for %s", (char *)id);

  // Connect to the websocket feed
  struct httpwss_session *coinbase_local;
  if (wss_client("ws-feed.pro.coinbase.com", "/", "443", &coinbase_local) !=
      WSS_ERR_NONE) {
    pprint_error(
        "%s@%s: %d unable to connect to coinbase websocket feed (aborting)",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  }

  // Subscribe to the feed
  _subscribe((char *)id, coinbase_local);

  // let the back buffer fill
  sleep(5);

  char *full_book_request = NULL;
  int full_book_request_len =
      snprintf(NULL, 0, "/products/%s/book?level=3", (char *)id);

  full_book_request = malloc((size_t)full_book_request_len + 1);
  sprintf(full_book_request, "/products/%s/book?level=3", (char *)id);

  coinbase_book *bid_book = NULL;
  coinbase_book *ask_book = NULL;

  char *buffer = NULL;
  struct httpwss_session *coinbase_rest =
      httpwss_session_new(COINBASE_API, "443");
  http_get_request(coinbase_rest, full_book_request, &buffer);

  free(full_book_request);
  __json_value full_book = json_parse(buffer);
  __json_object book = json_get_object(full_book);

  __json_value _sequence = hashmap_get("sequence", book);
  __json_array bids = json_get_array(hashmap_get("bids", book));
  __json_array asks = json_get_array(hashmap_get("asks", book));
  size_t snapshot_root = (size_t)(*(json_get_number(_sequence)));

  pprint_info("storing full order book for %s", (char *)id);
  bid_book = _build_book(bids);
  ask_book = _build_book(asks);
  pprint_info("finished full order book for %s", (char *)id);

  json_free(full_book);

  pprint_info("started normal message flow for %s", (char *)id);

  char *msg_rt = NULL;

  size_t start_time = (size_t)time(NULL);
  size_t end_time = (size_t)time(NULL);
  size_t num_msg = 0;

  while (globals_continue(NULL) &&
      wss_read_text(coinbase_local, &msg_rt) == WSS_ERR_NONE) {
    __json_value _msg = json_parse(msg_rt);
    __json_object msg = json_get_object(_msg);

    _sequence = hashmap_get("sequence", msg);
    size_t sequence = (size_t)(*(json_get_number(_sequence)));

    if (sequence <= snapshot_root) {
      json_free(_msg);
      free(msg_rt);
      num_msg += 1;
      continue;
    }

    if (sequence - snapshot_root != 1) {
      pprint_warn(
          "missed %lu messages in %s", (sequence - snapshot_root), (char *)id);
    } else if (sequence > (snapshot_root + 1)) {
      pprint_warn(
          "jumped through time by %lu message", (sequence - snapshot_root));
    }

    snapshot_root = sequence;

    __json_string msg_type = json_get_string(hashmap_get("type", msg));
    // each msg_type is identifiable by the first character of the message type
    switch (msg_type[0]) {
    case 'r': {
      // received
      _parse_received(msg, &bid_book, &ask_book);
      break;
    }
    case 'o': {
      // open
      _parse_open(msg, &bid_book, &ask_book);
      break;
    }
    case 'd': {
      // done
      _parse_done(msg, &bid_book, &ask_book);
      break;
    }
    case 'm': {
      // match
      _parse_match(msg, &bid_book, &ask_book);
      break;
    }
    default: {
      pprint_error(
          "%s@%s:%d im not supposed to get message type message_type=%s (aborting)",
          __FILE_NAME__, __func__, __LINE__, msg_type);
      abort();
    }
    }

    json_free(_msg);
    free(msg_rt);
    num_msg += 1;

    end_time = (size_t)time(NULL);

    if (end_time - start_time >= 1) {
      double msg_ps = (double)num_msg / (double)(end_time - start_time);
      pprint_info("%s %f msg/s", (char *)id, msg_ps);

      start_time = (size_t)time(NULL);
      end_time = (size_t)time(NULL);
      num_msg = 1;
    }

    /*
    struct coinbase_book_level bid_top;
    struct coinbase_book_level ask_top;
    coinbase_book_get(bid_book, BOOK_BID, 1, &bid_top);
    coinbase_book_get(bid_book, BOOK_ASK, 1, &ask_top);

    printf("%.2f %.8f\n", (double)(bid_top.level) / 100.0,
        (double)(bid_top.total) / 100000000.0);
        */
  }
}

void
exchanges_coinbase_init(void)
{
  char *full_products = NULL;

  struct httpwss_session *coinbase_rest =
      httpwss_session_new(COINBASE_API, "443");
  if (http_get_request(coinbase_rest, "/products", &full_products) != 0) {
    pprint_error("%s@%s:%d unable to get %s%s (aborting)", __FILE_NAME__,
        __func__, __LINE__, COINBASE_API, "/products");
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
