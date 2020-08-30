#include "exchanges.h"

static struct hashmap *securities = NULL;

void
exchange_init()
{
  securities = hashmap_new(1024);
}

void
exchange_put(char *name, struct security *sec)
{
  if (!securities) {
    pprint_error("%s@%s:%d must call exchange_init before adding exchange data "
                 "(aborting)",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  }
  hashmap_put(name, sec, securities);
}

struct security *
exchange_get(char *name)
{
  return hashmap_get(name, securities);
}
