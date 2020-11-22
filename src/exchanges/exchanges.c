#include "exchanges.h"

static struct hashmap *securities = NULL;

void
exchange_init()
{
  /*
   * TODO change the return value
   */
  hashmap_new(1024, &securities);
}

void
exchange_put(char *name, struct security *sec)
{
  if (!securities)
  {
    pprint_error("%s", "must call exchange_init before adding exchange data "
                       "(aborting)");
    abort();
  }
  hashmap_put(name, sec, securities);
}

struct security *
exchange_get(char *name)
{
  return hashmap_get(name, securities);
}

void
exchange_free()
{
  for (size_t i = 0; i < securities->num_bins; ++i)
  {
    struct _map_list *ll = securities->bins[i];
    while (ll)
    {
      struct _map_list *nxt = ll->next;
      security_free((struct security **)&(ll->value));
      ll = nxt;
    }
  }
  hashmap_free(securities);
}
