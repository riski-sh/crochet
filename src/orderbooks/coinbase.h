#ifndef ORDERBOOKS_COINBASE_H
#define ORDERBOOKS_COINBASE_H

#include <ffjson/ffjson.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "book.h"

/*
 * A linked list to store the order values this works because coinbase
 * operates on a FIFO basis
 */
struct coinbase_value {
  bool open;
  char _p1[7];
  uint64_t size;
  char *orderid;
  struct coinbase_value *nxt;
  struct coinbase_value *prv;
};

struct coinbase_book_level {
  uint64_t level;
  uint64_t total;
};

/*
 * Change the name of the generic orderbook to coinbase_book to
 * better code looking.
 */
typedef struct generic_book coinbase_book;

/*
 *
 * Primes the order for the order book
 *
 * @param book may be NULL if no book or the existing book to put.
 * @param price the price level to put.
 * @param e the order entry at level price
 */
void coinbase_book_received(
    coinbase_book **book, uint64_t price, struct coinbase_value *e);

/*
 * Gets a limited number of book entries. This will return the best
 * book entries from greatest to least or least to greatest depending
 * on the book_type. This will populate an array of the values found.
 *
 * @param book the book to extract values from.
 * @param book_type specifies weather book represents the bid or ask size
 * @param num the depth of the book to go.
 * @param data an array of size num to hold the book levels
 */
void coinbase_book_get(coinbase_book *book, book_t book_type, int num,
    struct coinbase_book_level *data);

void coinbase_book_open(
    coinbase_book **book, uint64_t price, uint64_t remaining, char *uuid);

void coinbase_book_remove(coinbase_book **book, uint64_t price, char *uuid);

void coinbase_book_match(
    coinbase_book **book, uint64_t price, uint64_t size, char *maker_id);

/*
 * Frees the coinbase_value structure.
 */
void coinbase_book_value_free(void *data);

#endif
