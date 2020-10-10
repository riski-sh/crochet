#include <globals/globals.h>

pthread_mutex_t lock;
bool is_safe = true;

bool
globals_continue(bool *val)
{
  if (val == NULL) {
    return is_safe;
  } else {
    pthread_mutex_lock(&lock);
    is_safe = *val;
    pthread_mutex_unlock(&lock);
    return is_safe;
  }
}
