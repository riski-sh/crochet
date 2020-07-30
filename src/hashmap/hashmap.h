#ifndef HASHMAP_H
#define HASHMAP_H

#include <pprint.h>
#include <stdlib.h>

/*
 * Private hashmap struct definition
 */
struct hashmap;

/*
 * Creates a new hashmap
 * @param num_bins the number of bins this hashmap to use
 * @return a newly created hashmap
 */
struct hashmap *hashmap_new(unsigned int num_bins);

/*
 * Frees the hashmap struct created by hashmap_new
 * @param map a pointer to the pointer to the hashmap
 */
void hashmap_free(struct hashmap **map);

/*
 * Puts a value in the hashmap
 * @param key a pointer to the key
 * @param value a pointer to the value
 * @param map the hashmap to put this value in
 */
void hashmap_put(const void *key, const void *value, struct hashmap *map);

/*
 * Gets a value out of the hashmap.
 * @param key the key to find
 * @param map the map to search for the key for
 * @return the value or null if the key doesn't exist
 */
const void *hashmap_get(const void *key, struct hashmap *map);

#endif
