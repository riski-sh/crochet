/*
 * The goal of this json library is to parse a json and be able to look up
 * information as fast as possible.
 */
#ifndef FFJSON_H
#define FFJSON_H

#include <hashmap/hashmap.h>
#include <pprint/pprint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct hashmap *__json_object;
typedef struct json_array *__json_array;
typedef double *__json_number;
typedef char *__json_string;
typedef struct json_value *__json_value;
typedef bool __json_bool;

enum JSON_TYPE {
  JSON_TYPE_OBJECT = 0, // expect hash map
  JSON_TYPE_ARRAY = 1, // expect a null terminated array
  JSON_TYPE_NUMBER = 2, // expect a double pointer
  JSON_TYPE_STRING = 3, // expect a const char *
  JSON_TYPE_TRUE = 4, // expect NULL
  JSON_TYPE_FALSE = 5, // expect NULL
  JSON_TYPE_NULL = 6, // expect NULL

  JSON_TYPE_NUM = 7 // not a json type just counts the number
                    // of json types
};

extern char *JSON_TYPE_STR[JSON_TYPE_NUM];

__json_object json_get_object(__json_value val);
__json_array json_get_array(__json_value val);
__json_number json_get_number(__json_value val);
__json_string json_get_string(__json_value val);
__json_bool json_get_bool(__json_value val);

struct json_value {
  enum JSON_TYPE t;

  // voluntary padding
  char _p[4];

  void *data;
};

struct json_array {
  __json_value val;
  __json_array nxt;
};

/*
 * Parses a json object
 * @param str the string representing a json
 */
__json_value json_parse(char *str);

/*
 * Takes an existing parse tree and given a json of the same structure
 * will instead just fill in the new values and not create an entirely
 * new json object. THIS IS HIGHLEY RECOMMENDED IF YOU ARE POLLING AND
 * RECEIVING THE SAME JSON OBJECT BACK.
 */
void json_parse_cached(char *str, size_t *idx, __json_value tree);

/*
 * Frees a json object and all its children
 * @param root the root of the object
 */
void json_free(__json_value root);

#endif
