#include "coinbase.h"

void
coinbase_book_received(
    coinbase_book **book, uint64_t price, struct coinbase_value *e)
{
  struct generic_book *b = book_query(book, price);
  if (b->price != price) {
    abort();
  }

  struct coinbase_value *v = b->data;
  if (v) {
    while (v->nxt) {
      v = v->nxt;
    }
    v->nxt = e;
    e->prv = v;
  } else {
    b->data = e;
  }

  // don't add the size to the total level yet since it is not
  // currently active in the book.
  // during normal message flow the e->open will always be false
  // during initial book making the open value will be true

  if (e->open) {
  } else {
    b->total += e->size;
  }
}

void
coinbase_book_open(
    coinbase_book **book, uint64_t price, uint64_t remaining, char *uuid)
{
  // find the level this order lives on
  struct generic_book *b = book_query(book, price);

  if (b->data == NULL) {
    pprint_error(
        "expected some data but found none", __FILE_NAME__, __func__, __LINE__);
    abort();
  }

  struct coinbase_value *v = b->data;
  while (v) {
    if (strcmp(v->orderid, uuid) == 0) {
      break;
    }
    v = v->nxt;
  }

  if (!v) {
    pprint_error("the order %s doesn't exist on price level %lu", __FILE_NAME__,
        __func__, __LINE__, uuid, price);
  }

  // make this order open
  v->open = true;
  v->size = remaining;
  b->total += v->size;
}

static void
_get_increasing(
    coinbase_book *book, int *cur, int num, struct coinbase_book_level *d)
{
  if (!book || *cur <= 0) {
    return;
  }
  _get_increasing(book->left, cur, num, d);

  if (!book || *cur <= 0) {
    return;
  }
  _get_increasing(book->right, cur, num, d);

  if (!book || *cur <= 0) {
    return;
  }

  d[num - (*cur)].total = book->total;
  d[num - (*cur)].level = book->price;

  *cur -= 1;
}

static void
_get_decreasing(
    coinbase_book *book, int *cur, int num, struct coinbase_book_level *d)
{
  if (!book || *cur <= 0) {
    return;
  }
  _get_decreasing(book->right, cur, num, d);

  if (!book || *cur <= 0) {
    return;
  }
  _get_decreasing(book->left, cur, num, d);

  if (!book || *cur <= 0) {
    return;
  }

  d[num - (*cur)].total = book->total;
  d[num - (*cur)].level = book->price;

  *cur -= 1;
}

void
coinbase_book_get(coinbase_book *book, book_t book_type, int num,
    struct coinbase_book_level *data)
{
  if (!book) {
    return;
  }

  switch (book_type) {
  case BOOK_BID:
    _get_decreasing(book, &num, num, data);
    break;
  case BOOK_ASK:
    _get_increasing(book, &num, num, data);
    break;
  }
}

void
coinbase_book_value_free(void *data)
{
  if (!data) {
    return;
  }

  struct coinbase_value *iter = data;
  while (iter) {
    struct coinbase_value *nxt = iter->nxt;
    free(iter->orderid);
    free(iter);
    iter = nxt;
  }
}
