#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <stdbool.h>

extern pthread_mutex_t lock;
extern bool is_safe;


/*
 * A global infinite loop boolean
 */
bool globals_continue(bool *val);

#endif
