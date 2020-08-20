#include "ffjson.h"

char *JSON_TYPE_STR[JSON_TYPE_NUM] = { "JSON_TYPE_OBJECT", "JSON_TYPE_ARRAY",
  "JSON_TYPE_NUMBER", "JSON_TYPE_STRING", "JSON_TYPE_TRUE", "JSON_TYPE_FALSE",
  "JSON_TYPE_NULL" };

// fwd declaration of the few functions needed to be recursive in the
// processing
static __json_value _parse_value(char *str, size_t *idx);

static void
_parse_whitespace(char *str, size_t *idx)
{
  while (str[*idx] == (char)0x20 || str[*idx] == (char)0x09 ||
      str[*idx] == (char)0x0A || str[*idx] == (char)0x0D) {
    (*idx) += 1;
  }
}

static bool
_parse_value_seperator(char *str, size_t *idx)
{
  _parse_whitespace(str, idx);
  if (str[*idx] == ',') {
    (*idx) += 1;
    _parse_whitespace(str, idx);
    return true;
  }
  return false;
}

static bool
_valid_character(char *str, size_t *idx)
{
  if (str[*idx] == '\\' &&
      (str[(*idx) + 1] == '"' || str[(*idx) + 1] == '\\' ||
          str[(*idx) + 1] == '/' || str[(*idx) + 1] == 'b' ||
          str[(*idx) + 1] == 'f' || str[(*idx) + 1] == 'n' ||
          str[(*idx) + 1] == 'r' || str[(*idx) + 1] == 't')) {
    (*idx) += 2;
    return true;
  }

  if (str[*idx] != '"') {
    (*idx) += 1;
    return true;
  }

  return false;
}

static __json_string
_parse_string(char *str, size_t *idx)
{
  if (str[*idx] != '"') {
    pprint_error("expected \" for string %s", __FILE_NAME__, __func__, __LINE__,
        &(str[*idx]));
    abort();
  }
  (*idx) += 1;

  char *str_begining = &(str[*idx]);

  // loop through all the characters in the string
  while (_valid_character(str, idx))
    ;

  // make sure we ended on a "
  if (str[*idx] != '"') {
    pprint_error("invalid json string expected \" got %s", __FILE_NAME__,
        __func__, __LINE__, &(str[*idx]));
    abort();
  }

  // Since this is a valid string some trickery will be here to avoid
  // creating a new string. Simply turn the ending " into a null character
  // and return a pointer to the beginning of the string.
  str[*idx] = '\x0';
  (*idx) += 1;
  return str_begining;
}

static void
_parse_member(char *str, size_t *idx, char **name, __json_value *_val)
{
  char *member_name = _parse_string(str, idx);

  // verify name-separator
  _parse_whitespace(str, idx);
  if (str[*idx] != ':') {
    pprint_error("expected a : after name %s", __FILE_NAME__, __func__,
        __LINE__, member_name);
  }
  (*idx) += 1;
  _parse_whitespace(str, idx);
  __json_value val = _parse_value(str, idx);

  *name = member_name;
  *_val = val;
}

static __json_object
_parse_object(char *str, size_t *idx)
{
  if (str[*idx] != '{') {
    pprint_info(
        "expected { got %s", __FILE_NAME__, __func__, __LINE__, &(str[*idx]));
    abort();
  }

  (*idx) += 1;

  // there is allowed white space after { so get rid of it
  _parse_whitespace(str, idx);

  // continue to parse members until no more members match
  // a member has a lookahead of "

  // 16 because R4stl1n said so
  __json_object obj = hashmap_new(16);
  if (str[*idx] == '"') {
    // loop through and parse all the members
    // we already know about the first member
    char *key;
    __json_value val;
    _parse_member(str, idx, &key, &val);
    hashmap_put(key, val, obj);
    while (_parse_value_seperator(str, idx)) {
      _parse_member(str, idx, &key, &val);
      hashmap_put(key, val, obj);
    }
  }

  // once all the members have been parsed (or 0) make sure we see the end
  // object

  _parse_whitespace(str, idx);
  if (str[*idx] != '}') {
    pprint_error("invalid object expected } got %s", __FILE_NAME__, __func__,
        __LINE__, &(str[*idx]));
    abort();
  }
  (*idx) += 1;

  // done processing the object.
  return obj;
}

static __json_array
_parse_array(char *str, size_t *idx)
{
  if (str[*idx] != '[') {
    pprint_info(
        "expected [ got %s", __FILE_NAME__, __func__, __LINE__, &(str[*idx]));
    abort();
  }

  (*idx) += 1;
  _parse_whitespace(str, idx);

  __json_array data = malloc(sizeof(struct json_array) * 1);

  // parse a value since one value must exist
  data->val = _parse_value(str, idx);

  __json_array iter = data;
  // parse the optional members of the array
  while (_parse_value_seperator(str, idx)) {
    // expand the array by 1
    iter->nxt = malloc(sizeof(struct json_array) * 1);
    iter = iter->nxt;
    iter->val = _parse_value(str, idx);
    iter->nxt = NULL;
  }
  _parse_whitespace(str, idx);

  // verify end array
  if (str[*idx] != ']') {
    pprint_error(
        "expected ] got %c", __FILE_NAME__, __func__, __LINE__, str[*idx]);
    abort();
  }
  (*idx) += 1;
  return data;
}

static __json_number
_parse_number(char *str, size_t *idx)
{
  // do the heavy lifting with strtod
  char *pend;
  double number = strtod(&(str[*idx]), &pend);

  // arrays are continuous, can perform pointer arithmetic to get how many
  // characters were read
  size_t characters = (size_t)(pend - (&str[*idx]));
  (*idx) += characters;

  __json_number n = (__json_number)malloc(sizeof(double) * 1);
  *n = number;

  return n;
}

static __json_value
_parse_value(char *str, size_t *idx)
{
  /*
   * A value is defined as on of these elements with there lookahead
   * below.
   *
   * object -> {
   * array -> [
   * string -> "
   * true -> true
   * false -> false
   * null -> null
   * number -> [ minus ] | zero | digit1-9
   */

  // all values get rid of the white space in front of them
  _parse_whitespace(str, idx);

  if (str[*idx] == '{') {
    // parse object
    __json_value val = malloc(sizeof(struct json_value) * 1);
    val->t = JSON_TYPE_OBJECT;
    val->data = _parse_object(str, idx);
    return val;
  } else if (str[*idx] == '[') {
    // parse array
    __json_value val = malloc(sizeof(struct json_value) * 1);
    val->t = JSON_TYPE_ARRAY;
    val->data = _parse_array(str, idx);
    return val;
  } else if (str[*idx] == '"') {
    // parse string
    __json_value val = malloc(sizeof(struct json_value) * 1);
    val->t = JSON_TYPE_STRING;
    val->data = _parse_string(str, idx);
    return val;
  } else if (str[*idx] == '-' || str[*idx] == '1' || str[*idx] == '2' ||
      str[*idx] == '3' || str[*idx] == '4' || str[*idx] == '5' ||
      str[*idx] == '6' || str[*idx] == '7' || str[*idx] == '8' ||
      str[*idx] == '9' || str[*idx] == '0') {
    // parse number
    __json_value val = malloc(sizeof(struct json_value) * 1);
    val->t = JSON_TYPE_NUMBER;
    val->data = _parse_number(str, idx);
    return val;
  } else if (strncmp(&(str[*idx]), "false", 5) == 0) {
    // parse false
    __json_value val = malloc(sizeof(struct json_value) * 1);
    val->t = JSON_TYPE_FALSE;
    val->data = NULL;
    (*idx) += 5;
    return val;
  } else if (strncmp(&(str[*idx]), "true", 4) == 0) {
    // parse true
    __json_value val = malloc(sizeof(struct json_value) * 1);
    val->t = JSON_TYPE_TRUE;
    val->data = NULL;
    (*idx) += 4;
    return val;
  } else if (strncmp(&(str[*idx]), "null", 4) == 0) {
    __json_value val = malloc(sizeof(struct json_value) * 1);
    val->t = JSON_TYPE_NULL;
    val->data = NULL;
    (*idx) += 4;
    return val;

  } else {
    pprint_error("expected object|array|string|true|false|null|number in json"
                 " %s %lu",
        __FILE_NAME__, __func__, __LINE__, &(str[*idx]), *idx);
    abort();
  }
}

__json_value
json_parse(char *str)
{
  if (!str) {
    return NULL;
  }

  // the current index of the string we are looking at
  size_t idx = 0;

  // ws value ws
  _parse_whitespace(str, &idx);
  __json_value val = _parse_value(str, &idx);
  return val;
}

#define _json_get(TYPE, VAL)                                                  \
  if (VAL->t != TYPE) {                                                       \
    pprint_error("expected TYPE found %s", __FILE_NAME__, __func__, __LINE__, \
        JSON_TYPE_STR[VAL->t]);                                               \
    abort();                                                                  \
  }                                                                           \
  return VAL->data

__json_object
json_get_object(__json_value val)
{
  _json_get(JSON_TYPE_OBJECT, val);
}

__json_array
json_get_array(__json_value val)
{
  _json_get(JSON_TYPE_ARRAY, val);
}

__json_number
json_get_number(__json_value val)
{
  _json_get(JSON_TYPE_NUMBER, val);
}

__json_string
json_get_string(__json_value val)
{
  _json_get(JSON_TYPE_STRING, val);
}

__json_bool
json_get_bool(__json_value val)
{
  if (val->t == JSON_TYPE_TRUE) {
    return true;
  } else if (val->t == JSON_TYPE_FALSE) {
    return false;
  } else {
    pprint_error("expected JSON_TYPE_TRUE or JSON_TYPE_FALSE got %s",
        __FILE_NAME__, __func__, __LINE__, JSON_TYPE_STR[val->t]);
    abort();
  }
}

void
json_free(__json_value root)
{
  switch (root->t) {
  case JSON_TYPE_OBJECT: {
    __json_object obj = root->data;
    for (size_t i = 0; i < obj->num_bins; ++i) {
      struct _map_list *ll = obj->bins[i];
      while (ll) {
        json_free(ll->value);
        ll = ll->next;
      }
    }
    hashmap_free(obj);
    free(root);
    break;
  }
  case JSON_TYPE_ARRAY: {
    __json_array arr = root->data;
    while (arr) {
      json_free(arr->val);
      __json_array nxt = arr->nxt;
      free(arr);
      arr = nxt;
    }
    free(root);
    break;
  }
  case JSON_TYPE_NUMBER: {
    free(root->data);
    free(root);
    break;
  }
  case JSON_TYPE_STRING: {
    free(root);
    break;
  }
  case JSON_TYPE_NUM:
    break;
  case JSON_TYPE_TRUE:
  case JSON_TYPE_FALSE:
  case JSON_TYPE_NULL: {
    free(root);
    break;
  }
  }
}
