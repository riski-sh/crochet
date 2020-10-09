#include <stdlib.h>

#include "http11.h"

#define STR_APPEND_STR(VAR, IDX, DATA)         \
  do {                                         \
    memcpy(&((VAR)[IDX]), DATA, strlen(DATA)); \
    IDX += strlen(DATA);                       \
  } while (0)

status_t
http11request_new(struct tls_session *session, struct http11request **_ret)
{
  /*
   * Make sure that the pointer is not allocated ie set to null
   */
  if (*_ret != NULL) {
    pprint_info("%s",
        "will not allocate a pointer that doesn't have a value "
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
  (*_ret)->session = session;
  (*_ret)->stub = NULL;

  return STATUS_OK;
}

static void
_http11request_read(struct tls_session *session, char *data)
{
  /*
   * Read through the headers
   *
   * if we see `connection: close` note to reconnect if attempting to send
   * again on this socket
   *
   * find either a Content-Length: or Transfer-Encoding to figure out how
   * to read the data
   */

  // Current index of the htp header
  int headeridx = 0;
  char header[MAX_SINGLE_HTTP_HEADER];

  /*
   * By default content_length is -1. if content_lenght > 0 then the server
   * has sent a Content-Length: XXX header. If content_length stays as -1 then
   * the server has sent Transfer-Encoding: Chunked header and therefore
   * the body of the response will be read differently.
   */
  int content_length = -1;
  bool found_content_length = false;

  /*
   * Set to true if the server has sent a Connect: Close
   */
  bool isclosed = false;
  bool found_connection_data = false;

  /*
   * SSL context to read from
   */
  SSL *ssl = session->ssl;

  /*
   * HTTP responses end in \r\n we will loop until we read that.
   * After that the body is read depending on how the body was sent
   */
  do {
    /*
     * Reset the index to the beginning
     */
    headeridx = 0;

    do {
      int ret = SSL_read(ssl, &(header[headeridx]), 1);
      if (ret == 1) {
        headeridx += 1;
      } else {
        pprint_error("expected 1 byte got SSL error %d", ret);
        abort();
      }
    } while (header[headeridx - 1] != '\n');

    header[headeridx] = '\x0';

    /*
     * Add a NULL character to allow for string processing
     */
    if (!found_content_length) {
      /*
       * Try to read the content length or transfer encoding
       */
      if (strncmp("Content-Length: ", header, 16) == 0) {
        content_length = strtod(&(header[16]), NULL);
        found_content_length = true;
      } else if (strncmp("Transfer-Encoding: ", header, 19)) {
        found_content_length = true;
      }
    }

    if (!found_connection_data) {
      /*
       * Look for a connection header
       */
      if (strncmp("Connection: close", header, 17) == 0) {
        found_connection_data = true;
        isclosed = true;
      } else if (strncmp("Connection: keep-alive", header, 22) == 0) {
        found_connection_data = true;
      }
    }
    (void)found_connection_data;

  } while ((header[0] != '\r' && header[1] != '\n'));

  /*
   * Check and make sure the connection data type header was found
   */
  if (!found_connection_data) {
    pprint_error("%s", "unable to find Connection header in HTTP response");
    abort();
  }

  /*
   * Make sure the content length header as found
   */
  if (!found_content_length) {
    pprint_error("%s", "unable to find content length in HTTP response");
    abort();
  }

  if (content_length == -1) {
    /*
     * Parse the chuncked response
     */
  } else {
    /*
     * Parse the response as a content length
     */
  }

  (void)content_length;
  (void)isclosed;
  (void)data;
}

static void
_http11request_cache(struct http11request *req)
{

  if (req->cache == NULL) {
    req->cache = malloc(MAX_HTTP_REQUEST_SIZE);
  }

  size_t cache_idx = 0;

  STR_APPEND_STR(req->cache, cache_idx, "GET ");
  STR_APPEND_STR(req->cache, cache_idx, req->stub);
  STR_APPEND_STR(req->cache, cache_idx, " HTTP/1.1\r\n");

  STR_APPEND_STR(req->cache, cache_idx, "Host: ");
  STR_APPEND_STR(req->cache, cache_idx, req->session->endpoint);
  STR_APPEND_STR(req->cache, cache_idx, "\r\n");

  uint64_t nbins = req->headers->num_bins;
  struct hashmap *headers = req->headers;

  for (uint64_t binidx = 0; binidx < nbins; ++binidx) {
    struct _map_list *bin = headers->bins[binidx];
    while (bin) {
      STR_APPEND_STR(req->cache, cache_idx, bin->orgkey);
      STR_APPEND_STR(req->cache, cache_idx, ": ");
      STR_APPEND_STR(req->cache, cache_idx, (char *)bin->value);
      STR_APPEND_STR(req->cache, cache_idx, "\r\n");
      bin = bin->next;
    }
  }

  STR_APPEND_STR(req->cache, cache_idx, "\r\n\x0");

  req->cache_len = strlen(req->cache);
}

status_t
http11request_push(struct http11request *req, char **_data)
{

  /*
   * Make sure a request exists and we didn't get a NULL pointer instead
   */
  if (req == NULL) {
    pprint_error("%s", "request can not be null");
    return STATUS_UNKNOWN_ERROR;
  }

  /*
   * Data must be a non null pointer and it is assumed to have an allocation of
   * MAX_HTTP_REQUEST_SIZE
   */
  // if (*_data == NULL) {
  //   pprint_error("%s", "data must be a non null value and is assumed to be
  //   allocated to MAX_HTTP_REQUEST_SIZE"); return STATUS_UNKNOWN_ERROR;
  // }

  /*
   * If this is dirty cache the request and remove the old cache
   */
  if (req->dirty) {
    _http11request_cache(req);
    pprint_warn(
        "\n"
        "==================================== cached ====================================\n"
        "%s"
        "================================================================================",
        req->cache);
    req->dirty = false;
  }

  /*
   * Send the data to the remote host
   */
  int ret = SSL_write(req->session->ssl, req->cache, req->cache_len);
  pprint_info("[DEBUG] sent %d bytes", ret);
  if (ret != req->cache_len) {
    /*
     * We didn't send all the data something whent wrong
     */
    pprint_error("attempted to send %d but only sent %d", req->cache_len, ret);
    return STATUS_UNKNOWN_ERROR;
  }

  /*
   * Read the response
   */
  // TODO
  _http11request_read(req->session, *_data);

  return STATUS_OK;
}
