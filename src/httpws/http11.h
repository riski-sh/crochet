#ifndef HTTP1_1_H
#define HTTP1_1_H

#include <stdlib.h>
#include <stdbool.h>
#include <hashmap/hashmap.h>
#include <status.h>
#include <pprint/pprint.h>
#include <string.h>

#include "session.h"

#define MAX_HTTP_REQUEST_SIZE 8192
#define MAX_SINGLE_HTTP_HEADER 2048

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

  /*
   * The endpoint to query. Example if you would like to get example.com/abc
   * the session would create a session to example.com and the stub would
   * point to /abc
   */
  char *stub;

  /*
   * The length of the cache
   */
  int cache_len;

  /*
   * The session to send this request on
   */
  struct tls_session *session;
};

/*
 * Creates a new http11request
 *
 * @param _ret allocated *ret and creates an empty get request by default
 * @param session
 */
status_t http11request_new(struct tls_session *session,
    struct http11request **_ret);

/*
 * Performs a request given an http11request
 *
 * @param req the request to perform
 * @param _data the response
 */
status_t http11request_push(struct http11request *req, char **_data);

#endif
