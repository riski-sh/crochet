#ifndef BOOK_H
#define BOOK_H

#include <pprint.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum { RANGE_INCREASING = 0, RANGE_DECREASING = 1 } range_t;

struct generic_book {
  struct generic_book *left;
  struct generic_book *right;
  struct generic_book *parent;

  uint64_t price;
  uint64_t total;
  void *data;
};

/*
 * Inserts a key into the AVL tree and returns the new node that represents
 * the tree. If the node already exists this will return the node with the
 * represented price query.
 *
 * @param root a pointer to the root pointer
 * @param price the price to find or insert
 * @return the node that represents this price level
 */
struct generic_book *book_query(struct generic_book **root, uint64_t price);

typedef void (*book_free_data)(void *_data);

/*
 * Frees an generic book and calls a book_free_data function to free
 * the generic data inside the node
 */
void book_free(struct generic_book *node, book_free_data free_func);

#endif
