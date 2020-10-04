#ifndef HASHMAP_H
#define HASHMAP_H

#include <pprint/pprint.h>
#include <stdint.h>
#include <stdlib.h>
#include <status.h>

/*
 *  Hash map struct is public to allow for users to define
 *  how they would like to iterate over it.
 */
struct hashmap {
  /*
   * an array of pointers that point to pointers of data
   */
  struct _map_list **bins;

  /*
   * store the number of bins to remember what to mod by
   * during the hash
   */
  uint64_t num_bins;

  /*
   * explicit padding
   */
  char _p1[8];
};

/*
 * the map list is kept public for user defined implementation
 * of how to handle the values
 */
struct _map_list {
  size_t key;
  void *value;
  struct _map_list *next;
};

/*
 * Creates a new hashmap
 * @param num_bins the number of bins this hashmap to use
 * @param _ret will allocate *_ret to a new hashmap struct
 * @return the status code
 */
status_t hashmap_new(uint64_t num_bins, struct hashmap **_ret);

/*
 * Frees the hashmap struct created by hashmap_new
 * @param map a pointer to the pointer to the hashmap
 */
void hashmap_free(struct hashmap *map);

/*
 * Puts a value in the hashmap
 * @param key a pointer to the key
 * @param value a pointer to the value
 * @param map the hashmap to put this value in
 */
void hashmap_put(char *key, void *value, struct hashmap *map);

/*
 * Gets a value out of the hashmap.
 * @param key the key to find
 * @param map the map to search for the key for
 * @return the value or null if the key doesn't exist
 */
void *hashmap_get(char *key, struct hashmap *map);

#endif
