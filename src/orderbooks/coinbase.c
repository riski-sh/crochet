#include "coinbase.h"

void
coinbase_book_put(
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
  } else {
    b->data = e;
  }
  b->total += e->size;
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
