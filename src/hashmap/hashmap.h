#ifndef HASHMAP_H
#define HASHMAP_H

#include <pprint.h>
#include <stdlib.h>

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
	unsigned int num_bins;

	/*
	 * explicit padding
	 */
	char _p1[4];
};

/*
 * the map list is kept public for user defined implementation
 * of how to handle the values
 */
struct _map_list {
	void *key;
	void *value;
	struct _map_list *next;
};

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
void hashmap_put(void *key, void *value, struct hashmap *map);

/*
 * Gets a value out of the hashmap.
 * @param key the key to find
 * @param map the map to search for the key for
 * @return the value or null if the key doesn't exist
 */
void *hashmap_get(void *key, struct hashmap *map);

#endif
