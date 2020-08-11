#ifndef ORDERBOOKS_COINBASE_H
#define ORDERBOOKS_COINBASE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "book.h"
#include <time.h>

struct coinbase_value {
  uint64_t size;
  char *orderid;
  struct coinbase_value *nxt;
};

typedef struct generic_book coinbase_book;

coinbase_book *coinbase_book_new(uint64_t price, struct coinbase_value *e);

void coinbase_book_put(
    coinbase_book **book, uint64_t price, struct coinbase_value *e);

void coinbase_book_print(coinbase_book *book, range_t order);

#endif
