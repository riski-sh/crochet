#ifndef STRING_STRING_H
#define STRING_STRING_H

#include <stdlib.h>

struct string_t {
  char *data;
  size_t len;
};

/*
 * Create a new string to work with
 *
 * @param string a string_t to initalize
 */
void string_new(struct string_t *string);

/*
 * Appends data to a string
 *
 * @param segment the substring to append to the string
 * @param len the length of the element that is to be appended
 */
void string_append(struct string_t *string, char *segment, size_t len);

#endif
