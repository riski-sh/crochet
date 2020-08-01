/*
 * The goal of this json library is to parse a json and be able to look up
 * information as fast as possible.
 */
#ifndef FFJSON_H
#define FFJSON_H

#include <pprint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum JSON_TYPE {
	JSON_TYPE_OBJECT = 0,
	JSON_TYPE_ARRAY = 1,
	JSON_TYPE_STRING = 2,
	JSON_TYPE_NUMBER = 3,
	JSON_TYPE_TRUE = 4,
	JSON_TYPE_FALSE = 5,
	JSON_TYPE_NULL = 6
};

struct json_element {
	enum JSON_TYPE type;

	// padding
	char _p[4];

	const char *key;
	struct json_element *value;
};

/*
 * Parses a json object
 * @param str the string representing a json
 */
struct json_element *json_parse(char *str);

#endif
