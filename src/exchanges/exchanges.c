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
  if (!securities) {
    pprint_error("%s",
        "must call exchange_init before adding exchange data "
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
