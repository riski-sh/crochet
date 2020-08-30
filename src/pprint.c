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

#if defined(__FreeBSD__)
  clock_gettime(CLOCK_REALTIME_PRECISE, &current_time);
#else
  clock_gettime(CLOCK_REALTIME, &current_time);
#endif

  sprintf(*time, "[\x1b[92m%lu.%09lu\x1b[0m]", current_time.tv_sec,
      current_time.tv_nsec);
}

void __attribute__((__format__(__printf__, 1, 0)))
pprint_info(const char *str, ...)
{
  char time[TIME_STR_LEN];
  _pprint_time(&time);

  _pprint_lock(stdout);
  printf("%s crochet: ", time);

  va_list args;
  va_start(args, str);
  vprintf(str, args);
  va_end(args);
  printf("\n");
  _pprint_unlock(stdout);
}

void __attribute__((__format__(__printf__, 1, 0)))
pprint_warn(const char *str, ...)
{
  char time[TIME_STR_LEN];
  _pprint_time(&time);

  _pprint_lock(stdout);
  printf("%s crochet: \x1b[93m", time);
  va_list args;
  va_start(args, str);
  vprintf(str, args);
  va_end(args);
  printf("\x1b[0m\n");
  _pprint_unlock(stdout);
}

void __attribute__((__format__(__printf__, 1, 0)))
pprint_error(const char *str, ...)
{
  char time[TIME_STR_LEN];
  _pprint_time(&time);

  _pprint_lock(stdout);
  printf("%s crochet: \x1b[91m", time);

  va_list args;
  va_start(args, str);
  vprintf(str, args);
  va_end(args);
  printf("\x1b[0m\n");
  _pprint_unlock(stdout);
}

#undef TIME_STR_LEN
