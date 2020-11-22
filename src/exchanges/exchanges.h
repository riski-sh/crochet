#ifndef EXCHANGES_H
#define EXCHANGES_H

#include <hashmap/hashmap.h>
#include <pprint/pprint.h>
#include <security/security.h>

void
exchange_init(void);

void
exchange_put(char *name, struct security *sec);

void
exchange_free(void);

struct security *
exchange_get(char *name);

#endif
