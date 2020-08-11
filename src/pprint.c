#include "pprint.h"

#define TIME_STR_LEN 35

/*
 * Locks a file for writing
 * @param fp the file to lock
 */
static inline void
_pprint_lock(FILE *fp)
{
  flockfile(fp);
}

/*
 * Unlocks a file for writing
 * @param fp the file to unlock
 */
static inline void
_pprint_unlock(FILE *fp)
{
  funlockfile(fp);
}

/*
 * Gets the time as a string in a standard format for logging.
 * @param time a place to put the time string in the given location.
 */
static void
_pprint_time(char (*time)[TIME_STR_LEN])
{
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);

  size_t epoch_usec = (size_t)(current_time.tv_sec * 1000) +
      (size_t)(current_time.tv_nsec / 1000000);

  sprintf(*time, "%lu", epoch_usec);
}

void __attribute__((__format__(__printf__, 1, 0)))
pprint_info(const char *str, const char *file, const char *func, int line, ...)
{
  char time[TIME_STR_LEN];
  _pprint_time(&time);

  _pprint_lock(stdout);
  printf("%s  INFO  %20s  %-20s  %04d  ", time, file, func, line);

  va_list args;
  va_start(args, line);
  vprintf(str, args);
  va_end(args);
  printf("\n");
  _pprint_unlock(stdout);
}

void __attribute__((__format__(__printf__, 1, 0)))
pprint_warn(const char *str, const char *file, const char *func, int line, ...)
{
  char time[TIME_STR_LEN];
  _pprint_time(&time);

  _pprint_lock(stdout);
  printf("%s  WARN  %-20s@%-20s:%4d  ", time, file, func, line);
  va_list args;
  va_start(args, line);
  vprintf(str, args);
  va_end(args);
  printf("\n");
  _pprint_unlock(stdout);
}

void __attribute__((__format__(__printf__, 1, 0)))
pprint_error(const char *str, const char *file, const char *func, int line, ...)
{
  char time[TIME_STR_LEN];
  _pprint_time(&time);

  _pprint_lock(stdout);
  printf("%s  ERRO  %20s  %-20s  %04d  ", time, file, func, line);

  va_list args;
  va_start(args, line);
  vprintf(str, args);
  va_end(args);
  printf("\n");
  _pprint_unlock(stdout);
}

#undef TIME_STR_LEN
