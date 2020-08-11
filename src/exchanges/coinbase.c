#include "coinbase.h"
#include "ffjson/ffjson.h"
#include "orderbooks/coinbase.h"

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
 */
static void
_coinbase_start(void *id)
{
  pprint_info(
      "starting exchange %s", __FILE_NAME__, __func__, __LINE__, (char *)id);

  FILE *test = fopen("full.json", "r");

  fseek(test, 0, SEEK_END);
  long length = ftell(test);
  fseek(test, 0, SEEK_SET);
  char *buffer = malloc((size_t)length);
  fread(buffer, 1, (size_t)length, test);
  fclose(test);

  pprint_info("storing full order book for %s", __FILE_NAME__, __func__,
      __LINE__, (char *)id);

  __json_value full_book = json_parse(buffer);
  __json_object book = json_get_object(full_book);

  __json_array bids = json_get_array(hashmap_get("bids", book));
  coinbase_book *bid_book = _build_book(bids);

  __json_array asks = json_get_array(hashmap_get("asks", book));
  coinbase_book *ask_book = _build_book(asks);

  pprint_info("finished full order book for %s", __FILE_NAME__, __func__,
      __LINE__, (char *)id);

  book_free(bid_book, coinbase_book_value_free);
  book_free(ask_book, coinbase_book_value_free);

  json_free(full_book);
  free(buffer);
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
