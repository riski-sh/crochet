#include "ffjson.h"

/* a mapping of the json enum type to its string equivelent */
char *JSON_TYPE_STR[JSON_TYPE_NUM] = { "JSON_TYPE_OBJECT", "JSON_TYPE_ARRAY",
  "JSON_TYPE_NUMBER", "JSON_TYPE_STRING", "JSON_TYPE_TRUE", "JSON_TYPE_FALSE",
  "JSON_TYPE_NULL" };

/* parse value is recursive and needs to have a forward declaration */
static __json_value _parse_value(char *str, size_t *idx);

/**
 * Parses whitespace characters that are valid in JSON
 *
 * @param str a pointer to the entire string that is being processed
 * @param idx the index of where to start, idx gets incremented and stops
 * incrementing when str[idx] is no longer a whitespace character
 */
static void
_parse_whitespace(char *str, size_t *idx)
{
  while (str[*idx] == ' ' || str[*idx] == '\n' || str[*idx] == '\t' ||
      str[*idx] == '\r') {
    (*idx) += 1;
  }
}

/**
 * Positions the idx pointer to the beginning of the value after the value
 * seperator.
 *
 * @param str a pointer  to the entire string that is being processed
 * @param idx the index of where to start, idx gets incrementd and stops
 * at the beginning of the value
 *
 * @return true if the value seperator (,) was found false if there is no
 * value seperator
 */
static bool
_parse_value_seperator(char *str, size_t *idx)
{

  /* parse the whitespace before the value */
  _parse_whitespace(str, idx);

  /*
   * determine if the value seperator denoted by a , exists after the
   * whitespace
   */
  if (str[*idx] == ',') {

    /* move past the comma */
    (*idx) += 1;

    /* continue skipping the whitespace until no more whitespace */
    _parse_whitespace(str, idx);

    return true;
  }
  return false;
}

static __json_string
_parse_string(char *str, size_t *idx)
{

  /* a string must begin with a " no matter what so make sure we start at it */
  if (str[*idx] != '"') {
    pprint_error(
        "expected \" but got %c while parsing a string (aborting)", str[*idx]);
    abort();
  }

  /* skip past the " since it isn't part of the string value */
  (*idx) += 1;

  /* mark the beginning on the string */
  char *str_begining = &(str[*idx]);

  /*
   * loop through all the characters until an unescped " comes to
   * mark the end of the string
   */
  while (str[*idx] != (char)'"' && str[(*idx) - 1] != (char)'\\') {
    (*idx) += 1;
  }

  /*
   * we only get here if we have reached the end of the string
   * therefore str[*idx] must equal " and str[*idx - 1] is not
   * equal to a back slash
   * Since this is a valid string some trickery will be here to avoid
   * creating a new string. Simply turn the ending " into a null character
   * and return a pointer to the beginning of the string.
   */
  str[*idx] = '\x0';

  /* increment one step past the null pointer that was just set */
  (*idx) += 1;

  /* return the in place string to save on lots of mallocs */
  return str_begining;
}

static void
_parse_member(char *str, size_t *idx, char **name, __json_value *_val,
    __json_object cached)
{
  char *member_name = _parse_string(str, idx);

  // verify name-separator
  _parse_whitespace(str, idx);
  if (str[*idx] != ':') {
    pprint_error("expected a : after name %s (aborting)", member_name);
    abort();
  }
  (*idx) += 1;
  _parse_whitespace(str, idx);

  if (!cached) {
    __json_value val = _parse_value(str, idx);
    *name = member_name;
    *_val = val;
  } else {
    __json_value val = hashmap_get(member_name, cached);
    json_parse_cached(str, idx, val);
  }
}

static __json_object
_parse_object(char *str, size_t *idx, __json_object cached)
{
  if (str[*idx] != '{') {
    pprint_error("expected { got %c (aborting)", __FILE_NAME__, str[*idx]);

    pprint_info("%s", &str[(*idx)]);
    abort();
  }

  (*idx) += 1;

  // there is allowed white space after { so get rid of it
  _parse_whitespace(str, idx);

  // continue to parse members until no more members match
  // a member has a lookahead of "

  __json_object obj = NULL;

  if (!cached) {
    // sorry R4stl1n 16 not a good number, 51 is tho ...
    //
    // TODO check this return value
    hashmap_new(51, &obj);

    if (str[*idx] == '"') {
      // loop through and parse all the members
      // we already know about the first member
      char *key;
      __json_value val;

      _parse_member(str, idx, &key, &val, NULL);
      hashmap_put(key, val, obj);
      while (_parse_value_seperator(str, idx)) {
        _parse_member(str, idx, &key, &val, NULL);
        hashmap_put(key, val, obj);
      }
    }
  } else {
    _parse_member(str, idx, NULL, NULL, cached);
    while (_parse_value_seperator(str, idx)) {
      _parse_member(str, idx, NULL, NULL, cached);
    }
  }

  // once all the members have been parsed (or 0) make sure we see the end
  // object

  _parse_whitespace(str, idx);
  if (str[*idx] != '}') {
    pprint_error("invalid object expected } got %s (aborting)", str[*idx]);
    abort();
  }
  (*idx) += 1;

  // done processing the object.
  if (!cached) {
    return obj;
  } else {
    return NULL;
  }
}

static __json_array
_parse_array(char *str, size_t *idx, __json_array cached)
{
  if (str[*idx] != '[') {
    pprint_info("expected [ got %s (aborting)", str[*idx]);
    abort();
  }

  (*idx) += 1;
  _parse_whitespace(str, idx);

  __json_array data = NULL;
  if (!cached) {
    data = calloc(1, sizeof(struct json_array));

    // the array could be empty if the array is empty than we should find
    // a ] character if we do not then we must parse a value
    if (str[*idx] != ']') {
      data->val = _parse_value(str, idx);

      __json_array iter = data;
      // parse the optional members of the array
      while (_parse_value_seperator(str, idx)) {
        // expand the array by 1
        iter->nxt = calloc(1, sizeof(struct json_array));
        iter = iter->nxt;
        iter->val = _parse_value(str, idx);
      }
      _parse_whitespace(str, idx);
    }
  } else {
    __json_array iter = cached;
    while (iter) {
      json_parse_cached(str, idx, iter->val);
      iter = iter->nxt;
      _parse_value_seperator(str, idx);
    }
  }

  // verify end array
  if (str[*idx] != ']') {
    pprint_error("expected ] got %c", str[*idx]);
    abort();
  }
  (*idx) += 1;

  if (!cached) {
    return data;
  } else {
    return NULL;
  }
}

static __json_number
_parse_number(char *str, size_t *idx, __json_number cached)
{
  // do the heavy lifting with strtod
  char *pend;
  double number = strtod(&(str[*idx]), &pend);

  // arrays are continuous, can perform pointer arithmetic to get how many
  // characters were read
  size_t characters = (size_t)(pend - (&str[*idx]));
  (*idx) += characters;

  if (!cached) {
    __json_number n = (__json_number)calloc(1, sizeof(double));
    *n = number;
    return n;
  } else {
    *cached = number;
    return NULL;
  }
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
    __json_value val = calloc(1, sizeof(struct json_value));
    val->t = JSON_TYPE_OBJECT;
    val->data = _parse_object(str, idx, NULL);
    return val;
  } else if (str[*idx] == '[') {
    // parse array
    __json_value val = calloc(1, sizeof(struct json_value));
    val->t = JSON_TYPE_ARRAY;
    val->data = _parse_array(str, idx, NULL);
    return val;
  } else if (str[*idx] == '"') {
    // parse string
    __json_value val = calloc(1, sizeof(struct json_value));
    val->t = JSON_TYPE_STRING;
    val->data = _parse_string(str, idx);
    return val;
  } else if (str[*idx] == '-' || str[*idx] == '1' || str[*idx] == '2' ||
      str[*idx] == '3' || str[*idx] == '4' || str[*idx] == '5' ||
      str[*idx] == '6' || str[*idx] == '7' || str[*idx] == '8' ||
      str[*idx] == '9' || str[*idx] == '0') {
    // parse number
    __json_value val = calloc(1, sizeof(struct json_value));
    val->t = JSON_TYPE_NUMBER;
    val->data = _parse_number(str, idx, NULL);
    return val;
  } else if (strncmp(&(str[*idx]), "false", 5) == 0) {
    // parse false
    __json_value val = calloc(1, sizeof(struct json_value));
    val->t = JSON_TYPE_FALSE;
    val->data = NULL;
    (*idx) += 5;
    return val;
  } else if (strncmp(&(str[*idx]), "true", 4) == 0) {
    // parse true
    __json_value val = calloc(1, sizeof(struct json_value));
    val->t = JSON_TYPE_TRUE;
    val->data = NULL;
    (*idx) += 4;
    return val;
  } else if (strncmp(&(str[*idx]), "null", 4) == 0) {
    __json_value val = calloc(1, sizeof(struct json_value));
    val->t = JSON_TYPE_NULL;
    val->data = NULL;
    (*idx) += 4;
    return val;
  } else {
    pprint_error("expected object|array|string|true|false|null|number in json"
                 "%s %lu (aborting)",
        &(str[*idx]), *idx);
    abort();
  }
}

__json_value
json_parse(char *str)
{
  if (!str) {
    pprint_error("%s", "no string given to parse (aborting)");
    abort();
  }

  // the current index of the string we are looking at
  size_t idx = 0;

  // ws value ws
  _parse_whitespace(str, &idx);
  __json_value val = _parse_value(str, &idx);
  return val;
}

void
json_parse_cached(char *str, size_t *idx, __json_value tree)
{
  // ws value ws
  _parse_whitespace(str, idx);

  if (!tree ||
      (tree->t != JSON_TYPE_TRUE && tree->t != JSON_TYPE_FALSE &&
          tree->t != JSON_TYPE_NULL && tree->data == NULL)) {
    pprint_error("%s",
        "the json received is not the same and therefore cannot be cache parsed (aborting)");
    abort();
  }

  switch (tree->t) {
  case JSON_TYPE_OBJECT:
    _parse_object(str, idx, tree->data);
    break;
  case JSON_TYPE_ARRAY:
    _parse_array(str, idx, tree->data);
    break;
  case JSON_TYPE_NUMBER:
    _parse_number(str, idx, tree->data);
    break;
  case JSON_TYPE_STRING:
    tree->data = _parse_string(str, idx);
    break;
  case JSON_TYPE_TRUE:
  case JSON_TYPE_FALSE:
  case JSON_TYPE_NULL:
    if (strncmp(&(str[*idx]), "false", 5) == 0) {
      tree->t = JSON_TYPE_FALSE;
      *idx += 5;
    } else if (strncmp(&(str[*idx]), "true", 4) == 0) {
      tree->t = JSON_TYPE_TRUE;
      *idx += 4;
    } else if (strncmp(&(str[*idx]), "null", 4) == 0) {
      tree->t = JSON_TYPE_NULL;
      *idx += 4;
    }
    break;
  case JSON_TYPE_NUM:
    pprint_error("%s", "invalid json type while parsing cached tree");
    abort();
  }
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
    pprint_error("expected JSON_TYPE_TRUE or JSON_TYPE_FALSE got %s (aborting)",
        JSON_TYPE_STR[val->t]);
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
    if (arr->val != NULL) {
      while (arr) {
        json_free(arr->val);
        __json_array nxt = arr->nxt;
        free(arr);
        arr = nxt;
      }
    } else {
      free(arr);
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
