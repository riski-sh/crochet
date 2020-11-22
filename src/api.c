#include <api.h>

static struct chart_object *
chart_object_new()
{
  struct chart_object *obj = NULL;
  obj = malloc(sizeof(struct chart_object) * 1);

  /* verify malloc allocted memory */
  if (!obj) {
    pprint_error(
        "unable to allocate %lu bytes (aborting)", sizeof(struct chart_object));
    exit(1);
  }
  return obj;
}

void
chart_create_object_line(struct candle *cnd, size_t start_idx,
    uint32_t start_price, size_t end_idx, uint32_t end_price,
    const char *function_name)
{
  /* create a generic object */
  struct chart_object *obj = chart_object_new();

  /* set the shortname */
  obj->name = function_name;

  /* set the type of the containing object */
  obj->object_type = CHART_OBJECT_LINE;

  /* make this the to be first element in the list */
  obj->next = cnd->analysis_list;

  struct chart_object_t_line *type = NULL;
  type = malloc(sizeof(struct chart_object_t_line) * 1);

  /* verify memory is good */
  if (!type) {
    pprint_error("unable to allocate %lu bytes (aborting)",
        sizeof(struct chart_object_t_text));
    exit(1);
  }

  /* the start and end of the line */
  type->start = start_idx;
  type->start_price = start_price;
  type->end = end_idx;
  type->end_price = end_price;

  obj->value = (void *)type;

  /* make the newly created object the root of the list */
  cnd->analysis_list = obj;
}

void
chart_create_object_text(struct candle *cnd, char c, const char *function_name)
{
  /* create a generic object */
  struct chart_object *obj = chart_object_new();

  /* set the shortname to understand who created this analysis */
  obj->name = function_name;

  /* we will add this analysis the beginning of the linked list */
  obj->next = cnd->analysis_list;

  /* define the object type to make sure readers cast the void* correctly */
  obj->object_type = CHART_OBJECT_TEXT;

  /* create a text object */
  struct chart_object_t_text *type = NULL;
  type = malloc(sizeof(struct chart_object_t_text) * 1);

  /* verify malloc allocated memory */
  if (!type) {
    pprint_error("unable to allocate %lu bytes (aborting)",
        sizeof(struct chart_object_t_text));
    exit(1);
  }

  /* set the character to be displayed at the bottom of the candle */
  type->TEXT = c;

  /* point the generic object to the typed object */
  obj->value = (void *)type;

  /* make the newly created object the root of the list */
  cnd->analysis_list = obj;
}
