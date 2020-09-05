#include "hashmap.h"

static uint32_t
sdbm(char *str)
{
  uint32_t hash = 0;
  int c;

  while ((c = *str++) && c != 0)
    hash = (uint32_t)c + (hash << 6) + (hash << 16) - hash;

  return hash;
}

static void
_map_list_add(struct _map_list **list, size_t ori_key, void *value)
{

  struct _map_list *val = calloc(1, sizeof(struct _map_list));
  val->next = *list;
  *list = val;
  val->key = ori_key;
  val->value = value;
}

struct hashmap *
hashmap_new(unsigned int num_bins)
{
  struct hashmap *map = calloc(1, sizeof(struct hashmap));
  if (!map) {
    pprint_error("%s@%s:%d not enough memory (aborting)", __FILE_NAME__,
        __func__, __LINE__);
    abort();
  }
  map->bins = calloc(num_bins, sizeof(struct _map_list));
  if (!map->bins) {
    pprint_error("%s@%s:%d not enough memory (aborting)", __FILE_NAME__,
        __func__, __LINE__);
    abort();
  }
  for (unsigned int i = 0; i < num_bins; ++i) {
    map->bins[i] = NULL;
  }
  map->num_bins = num_bins;
  return map;
}

void
hashmap_put(char *key, void *value, struct hashmap *map)
{
  if (!key || !value || !map) {
    pprint_error("%s@%s:%d key, value, or map points to null (aborting)",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  }
  uint32_t hash = sdbm(key);
  uint32_t bin = hash % map->num_bins;

  struct _map_list **ll = &(map->bins[bin]);
  _map_list_add(ll, hash, value);
}

void *
hashmap_get(char *key, struct hashmap *map)
{
  uint32_t hash = sdbm(key);
  uint32_t bin = hash % map->num_bins;

  struct _map_list *iter = map->bins[bin];

  while (iter) {
    if (iter->key == hash) {
      return iter->value;
    }
    iter = iter->next;
  }

  // key doesn't exist
  return NULL;
}

void
hashmap_free(struct hashmap *map)
{
  for (size_t i = 0; i < map->num_bins; ++i) {
    struct _map_list *ll = map->bins[i];
    while (ll) {
      struct _map_list *nxt = ll->next;
      free(ll);
      ll = nxt;
    }
  }
  free(map->bins);
  free(map);
}
