#ifndef HTTP1_1_H
#define HTTP1_1_H

#include <stdlib.h>
#include <stdbool.h>
#include <hashmap/hashmap.h>
#include <status.h>
#include <pprint/pprint.h>

#include "session.h"

/*
 * List of supported request types
 */
typedef enum {
  HTTPREQ_GET,
} httpreq_t;

/*
 * A representation of an http 1.1 request.
 */
struct http11request {

  /*
   * The http request type
   */
  httpreq_t type;

  /*
   * The headers to send
   */
  struct hashmap *headers;

  /*
   * Any data that is going along with the request if the request is of type
   * POST. If a GET request data should be NULL
   */
  char *data;

  /*
   * Cached string
   */
  char *cache;

  /*
   * If dirty is set to true the cache will be regenerated before sending data.
   * If dirty is false then the request string will not be regenerated and
   * the value of cache will be sent instead.
   */
  bool dirty;
};

/*
 * Creates a new http11request
 *
 * @param _ret allocated *ret and creates an empty get request by default
 */
status_t http11request_new(struct http11request **_ret);

#endif
