#include "coinbase.h"

void
coinbase_book_match(
    coinbase_book **book, uint64_t price, uint64_t size, char *maker_id)
{
  struct generic_book *b = book_query(book, price);
  struct coinbase_value *v = b->data;

  while (v) {
    if (strcmp(maker_id, v->orderid) == 0) {
      break;
    }
    v = v->nxt;
  }

  if (!v) {
    pprint_error("the order %s doesn't exist on price level %lu", __FILE_NAME__,
        __func__, __LINE__, maker_id, price);
    abort();
  }

  v->size -= size;
  b->total -= size;
}

void
coinbase_book_remove(coinbase_book **book, uint64_t price, char *uuid)
{
  struct generic_book *b = book_query(book, price);
  struct coinbase_value *v = b->data;

  while (v) {
    if (strcmp(uuid, v->orderid) == 0) {
      break;
    }
    v = v->nxt;
  }

  if (!v) {
    pprint_error("the order %s doesn't exist on price level %lu", __FILE_NAME__,
        __func__, __LINE__, uuid, price);
    // what doesn't exist i guess we don't have to delete
    abort();
  }

  if (v->prv) {
    v->prv->nxt = v->nxt;
    if (v->nxt) {
      v->nxt->prv = v->prv;
    }
  } else {
    b->data = v->nxt;
    if (v->nxt) {
      v->nxt->prv = NULL;
    }
  }

  if (v->open) {
    b->total -= v->size;
  }

  // free the individual node
  free(v->orderid);
  free(v);

  if (b->total == 0 && b->data == NULL) {
    book_remove(book, b->price, coinbase_book_value_free);
  }
}

void
coinbase_book_received(
    coinbase_book **book, uint64_t price, struct coinbase_value *e)
{
  struct generic_book *b = book_query(book, price);
  e->prv = NULL;
  e->nxt = NULL;

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
    b->total += e->size;
  }
}

void
coinbase_book_open(
    coinbase_book **book, uint64_t price, uint64_t remaining, char *uuid)
{
  // find the level this order lives on
  struct generic_book *b = book_query(book, price);
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
    abort();
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
  _get_increasing(book->right, cur, num, d);

  if (!book || *cur <= 0) {
    return;
  }

  d[num - (*cur)].total = book->total;
  d[num - (*cur)].level = book->price;

  *cur -= 1;

  if (!book || *cur <= 0) {
    return;
  }
  _get_increasing(book->left, cur, num, d);
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

  d[num - (*cur)].total = book->total;
  d[num - (*cur)].level = book->price;

  *cur -= 1;

  if (!book || *cur <= 0) {
    return;
  }
  _get_decreasing(book->left, cur, num, d);
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
