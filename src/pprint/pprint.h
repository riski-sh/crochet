/*
 * Pprint is an abstraction of printf that allows for safe multithreaded
 * output to stdout and stderr. It is also provides a useful logging interface,
 * as well as progress bars and other useful features for logging.
 *
 * Log Format
 * The log format is a variation of log4j format structure. The format is as
 * follows.
 *
 * seconds.microseconds \t level \t file@function:line \t message
 *
 * This format is easily generated in C and allows for basic reflection at
 * where the log was generated at in code.
 */

#ifndef PPRINT_H
#define PPRINT_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(__clang__) || !defined(__llvm__)
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#endif

/*
 * Prints out a log message under the level info
 * @param str A constant string to print
 */
void
_pprint_info(const char *file, const char *func, const int line,
             const char *str, ...);

/*
 * Prints out a log message under the level warning
 * @param str A constant string to print
 */
void
_pprint_warn(const char *file, const char *func, const int line,
             const char *str, ...);

/*
 * Prints out a log message under the level error
 * @param str A constant string to print
 */
void
_pprint_error(const char *file, const char *func, const int line,
              const char *str, ...);

#define PPRINT_CHECK_ALLOC(PTR)                                                \
  do                                                                           \
  {                                                                            \
    if (!PTR)                                                                  \
    {                                                                          \
      pprint_error("%s@%s:%d %s == NULL *alloc failed", __FILE__, __func__,    \
                   __LINE__, #PTR);                                            \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define pprint_info(FMT, ...)                                                  \
  _pprint_info(__FILE__, __func__, __LINE__, FMT, __VA_ARGS__)

#define pprint_warn(FMT, ...)                                                  \
  _pprint_warn(__FILE__, __func__, __LINE__, FMT, __VA_ARGS__)

#define pprint_error(FMT, ...)                                                 \
  _pprint_error(__FILE__, __func__, __LINE__, FMT, __VA_ARGS__)

#endif
