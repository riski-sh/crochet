/*
 * The goal of this json library is to parse a json and be able to look up
 * information as fast as possible.
 */
#ifndef FFJSON_H
#define FFJSON_H

#include <hashmap/hashmap.h>
#include <pprint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct hashmap *__json_object;
typedef struct json_array *__json_array;
typedef double *__json_number;
typedef char *__json_string;
typedef struct json_value *__json_value;

enum JSON_TYPE {
	JSON_TYPE_OBJECT = 0, // expect hash map
	JSON_TYPE_ARRAY = 1, // expect a null terminated array
	JSON_TYPE_NUMBER = 2, // expect a double pointer
	JSON_TYPE_STRING = 3, // expect a const char *
	JSON_TYPE_TRUE = 4, // expect NULL
	JSON_TYPE_FALSE = 5, // expect NULL
	JSON_TYPE_NULL = 6 // expect NULL
};

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
[
  {
     "precision": "zip",
     "Latitude":  37.7668,
     "Longitude": -122.3959,
     "Address":   "",
     "City":      "SAN FRANCISCO",
     "State":     "CA",
     "Zip":       "94107",
     "Country":   "US"
  },
  {
     "precision": "zip",
     "Latitude":  37.371991,
     "Longitude": -122.026020,
     "Address":   "",
     "City":      "SUNNYVALE",
     "State":     "CA",
     "Zip":       "94085",
     "Country":   "US"
  }
]
*/
#endif
