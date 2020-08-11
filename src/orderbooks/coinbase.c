#include "coinbase.h"
#include "orderbooks/book.h"

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
_print_increasing(coinbase_book *book)
{
  (void) book;
}

static void
_print_decreasing(coinbase_book *book, int lvl)
{
  if (!book || lvl <= 0) {
    return;
  }
  _print_decreasing(book->right, lvl);
  printf("%f\t%10lu\n", ((double)book->price)/100.0, book->total);
  _print_decreasing(book->left, lvl-1);
}

void
coinbase_book_print(coinbase_book *book, range_t order)
{
  switch (order) {
  case RANGE_INCREASING:
    _print_increasing(book);
    break;
  case RANGE_DECREASING:
    _print_decreasing(book, 1);
    break;
  }
}

void
coinbase_book_value_free(void* data)
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
