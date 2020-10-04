#include "http11.h"

status_t
http11request_new(struct http11request **_ret)
{
  /*
   * Make sure that the pointer is not allocated ie set to null
   */
  if (*_ret != NULL) {
    pprint_info("%s", "will not allocate a pointer that doesn't have a value "
                      "of null");
    return STATUS_EXPECTED_NULL;
  }

  /*
   * Allocate 1 http11request and set its default values
   */
  *_ret = malloc(sizeof(struct http11request) * 1);
  (*_ret)->cache = NULL;
  (*_ret)->data = NULL;

  /*
   * Dirty is defaulted to true to make sure that the first request is cached
   */
  (*_ret)->dirty = true;

  /*
   * Attempt to get a new hashmap
   */
  struct hashmap *headers = NULL;
  status_t ret;
  if ((ret = hashmap_new(16, &headers)) && ret != STATUS_OK) {
    pprint_error("%s", "");
    return ret;
  }

  (*_ret)->headers = headers;
  (*_ret)->type = HTTPREQ_GET;
  return STATUS_OK;
}
