#ifndef SECURITY_ANALYSIS_H
#define SECURITY_ANALYSIS_H

#include <api.h>
#include <dirent.h>
#include <dlfcn.h>
#include <lib.h>
#include <pprint/pprint.h>
#include <stdbool.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/*
 * Recursively searches through base_path for .so files and loads them into
 * the list of analysis to run
 *
 * @param base_path the path to search for libs
 */
void
analysis_init(char *base_path);

/*
 * Runs all the analysis that are currently loaded
 *
 * @param cnds the candle list to pass to the functions
 * @param indx the index to pass to the candles
 */
void
analysis_run(struct candle *cnds, size_t indx);

/*
 * Clears the loaded analysis table
 */
void
analysis_clear(void);

#endif
