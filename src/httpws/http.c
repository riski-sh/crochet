#include <sys/poll.h>

#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/sslerr.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http.h"

#define HTTP_WSS_UPGRADE_FMT      \
  "GET %s HTTP/1.1\r\n"           \
  "Host: %s\r\n"                  \
  "Upgrade: websocket\r\n"        \
  "Connection: Upgrade\r\n"       \
  "Sec-WebSocket-Key: %s\r\n"     \
  "Sec-WebSocket-Version: 13\r\n" \
  "\r\n"

#define HTTP_GET_REQUEST_FMT \
  "GET %s HTTP/1.0\r\n"      \
  "Host: %s\r\n"             \
  "User-Agent: crochet\r\n"  \
  "\r\n"

#define HTTP_GET_REQUEST_AUTH_FMT    \
  "GET %s HTTP/1.1\r\n"              \
  "Host: %s\r\n"                     \
  "User-Agent: crochet\r\n"          \
  "Accept-Datetime-Format: UNIX\r\n" \
  "Authorization: Bearer %s\r\n"     \
  "\r\n"

struct _http_response {
  char *header_name;
  char *header_value;

  struct _http_response *next;
};

enum _http_parse_state {
  _http_parse_state_init = 0,
  _http_parse_state_header = 1,
  _http_parse_state_value = 2,
  _http_parse_state_end = 3
};

typedef enum { CONTENT_LENGTH = 0, CHUNCKED = 1, UNKNOWN = 2 } data_t;

// static struct _http_response *_http_parse_response(char **res);
// static void _http_response_free(struct _http_response *res);

static void
_http_ssl_selectrfd(SSL *ssl)
{
  fd_set fds;
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  int sock = SSL_get_rfd(ssl);
  FD_ZERO(&fds);
  FD_SET(sock, &fds);

  int err = select(sock + 1, &fds, NULL, NULL, &tv);
  if (err > 0) {
    return;
  }

  if (err == 0) {
    // timeout...
    pprint_error("connection time out");
    abort();
  } else {
    // error...
    pprint_error("select error in SSL_ERROR_WANT_READ");
    abort();
  }
}

static void
_http_ssl_selectwfd(SSL *ssl)
{
  fd_set fds;
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  int sock = SSL_get_wfd(ssl);
  FD_ZERO(&fds);
  FD_SET(sock, &fds);

  int err = select(sock + 1, NULL, &fds, NULL, &tv);
  if (err > 0) {
    return;
  }

  if (err == 0) {
    // timeout...
    pprint_error("connection time out");
    abort();
  } else {
    // error...
    pprint_error("select error in SSL_ERROR_WANT_READ");
    abort();
  }
}

static void
_http_ssl_read_header(SSL *ssl, char **name, char **value)
{
  /*
   * Holds the header line, this client can not accept headers greater than
   * 8kb
   */
  static char header[8 * 1024] = { 0 };

  int index = 0;
  int ssl_read_err = 0;

  bool set_name = false;

  do {
    while ((ssl_read_err = SSL_read(ssl, &(header[index]), 1)) &&
        ssl_read_err <= 0) {
      int err = SSL_get_error(ssl, ssl_read_err);
      switch (err) {
      case SSL_ERROR_WANT_READ:
        _http_ssl_selectrfd(ssl);
        continue;
      case SSL_ERROR_WANT_WRITE:
        _http_ssl_selectwfd(ssl);
        continue;
      default:
        pprint_error("unknown error\n");
      }
    }
    if (header[index - 1] == ':') {
      header[index - 1] = '\x0';
      *value = &(header[index]);
      *name = header;
      set_name = true;
    }
    index += 1;
  } while (header[index - 1] != '\n');

  if (!set_name) {
    *value = header;
    *name = header;
  }

  header[index-2] = '\x0';
}

/*
 * Reads the entire response
 *
 * @param ssl the ssl to read from
 * @param response a pointer to some buffer to hold the response. this buffer
 * may be reused to avoid calling realloc.
 * @param len the total length this buffer can hold
 */
static void
_http_ssl_read_all(SSL *ssl, char **response, size_t *len)
{
  (void)response;
  (void)len;

  char *header_name = NULL;
  char *header_value = NULL;

  bool found_keep_alive = false;
  bool found_payload_type = false;

  bool keep_alive = false;
  bool is_chuncked = false;

  size_t content_length = 0;

  do {
    _http_ssl_read_header(ssl, &header_name, &header_value);

    if (!found_keep_alive) {
      if (strcmp(header_name, "Connection") == 0) {
        found_keep_alive = true;
        if (strcmp(header_value, "keep-alive") == 0) {
          keep_alive = true;
        }
      }
    }

    if (!found_payload_type) {
      if (strcmp(header_name, "Content-Length") == 0) {
        content_length = strtoul(header_value, NULL, 10);
        found_payload_type = true;
      } else if (strcmp(header_name, "Transfer-Encoding") == 0 &&
          strcmp(header_value, " chunked") == 0) {
        is_chuncked = true;
        found_payload_type = true;
      }
    }

  } while (header_name[0] != '\x0' && header_value[0] != '\x0');

  if (is_chuncked) {

    char *chunk_len = NULL;
    size_t total_length = 0;
    size_t total_read = 0;

    _http_ssl_read_header(ssl, &chunk_len, &chunk_len);
    size_t chunk_len_d = strtoul(chunk_len, NULL, 16);

    while (chunk_len_d != 0) {
      total_length += chunk_len_d;

      if (total_length >= *len) {
        *response = realloc(*response, total_length * 2 * sizeof(char));

        if (!(*response)) {
          pprint_error("unable to allocate %lu", total_length * 2);
        }

        *len = total_length * 2;
      }

      int ssl_ret = 0;
      while ((size_t)total_read != total_length &&
          (ssl_ret = SSL_read(ssl, &((*response)[total_read]),
               (int)total_length - (int)total_read))) {
        if (ssl_ret > 0) {
          total_read += (size_t)ssl_ret;
        } else {
          int err = SSL_get_error(ssl, ssl_ret);
          switch (err) {
          case SSL_ERROR_WANT_READ:
            _http_ssl_selectrfd(ssl);
            continue;
          case SSL_ERROR_WANT_WRITE:
            _http_ssl_selectwfd(ssl);
            continue;
          default:
            pprint_error("unknown error\n");
          }
        }
      }

      (*response)[total_read] = '\x0';

      _http_ssl_read_header(ssl, &chunk_len, &chunk_len);
      _http_ssl_read_header(ssl, &chunk_len, &chunk_len);
      chunk_len_d = strtoul(chunk_len, NULL, 16);
    }
    _http_ssl_read_header(ssl, &chunk_len, &chunk_len);
  } else {
    pprint_warn("processing full payload of length %lu", content_length);
    if (content_length >= (*len)) {
      *response = realloc(*response, (content_length + 1) * sizeof(char));
      *len = (content_length);
    }

    int ssl_ret = 0;
    int read = 0;
    while ((size_t)read != content_length &&
        (ssl_ret = SSL_read(
             ssl, &((*response)[read]), (int)content_length - (int)read))) {
      if (ssl_ret > 0) {
        read += (size_t)ssl_ret;
      } else {
        int err = SSL_get_error(ssl, read);
        switch (err) {
        case SSL_ERROR_WANT_READ:
          _http_ssl_selectrfd(ssl);
          continue;
        case SSL_ERROR_WANT_WRITE:
          _http_ssl_selectwfd(ssl);
          continue;
        default:
          pprint_error("unknown error\n");
        }
      }
    }
    (*response)[content_length] = '\x0';
  }
}

int
http_wss_upgrade(struct httpwss_session *session, char *path)
{

  /*
   * example http upgrade request
   *
   * GET /chat HTTP/1.1
   * Host: server.example.com
   * Upgrade: websocket
   * Connection: Upgrade
   * Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
   * Sec-WebSocket-Protocol: chat, superchat
   * Sec-WebSocket-Version: 13
   */

  // Generate a 16 byte random number to be used as a communication feed.
  unsigned char raw_key[HTTP_WSS_KEY_LEN];

#ifdef _WIN32
#pragma error windows has no / dev / urandom.
#endif

  FILE *urandom = fopen("/dev/urandom", "rb");
  for (size_t i = 0; i < HTTP_WSS_KEY_LEN; ++i) {
    raw_key[i] = (unsigned char)fgetc(urandom);
  }
  fclose(urandom);

  pprint_info("generated crypto safe key "
              "0x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x",
      raw_key[0], raw_key[1], raw_key[2], raw_key[3], raw_key[4], raw_key[5],
      raw_key[6], raw_key[7], raw_key[8], raw_key[9], raw_key[10], raw_key[11],
      raw_key[12], raw_key[13], raw_key[14], raw_key[15]);

  unsigned char *key_encoded = NULL;
  key_encoded = base64(raw_key, HTTP_WSS_KEY_LEN);

  pprint_info("generated Sec-WebSocket-Key: %s", key_encoded);

  int req_size = snprintf(
      NULL, 0, HTTP_WSS_UPGRADE_FMT, path, session->endpoint, key_encoded);

  char *request = calloc(((unsigned long)req_size + 1), sizeof(char));
  sprintf(request, HTTP_WSS_UPGRADE_FMT, path, session->endpoint, key_encoded);

  // don't send over the NULL terminator
  SSL_write(session->ssl, request, req_size);

  free(request);

  // get the response headers with values and find the
  // Sec-WebSocket-Accept header.

  char *response = NULL;
  size_t test = 0;

  _http_ssl_read_all(session->ssl, &response, &test);

  free(key_encoded);
  session->iswss = true;
  return 0;
}

void
http_get_request(struct httpwss_session *session, char *path, char **response)
{
  int req_size;
  char *request;

  if (session->hashauth) {
    req_size = snprintf(NULL, 0, HTTP_GET_REQUEST_AUTH_FMT, path,
        session->endpoint, session->authkey);
    request = calloc(((unsigned long)req_size + 1), sizeof(char));
    sprintf(request, HTTP_GET_REQUEST_AUTH_FMT, path, session->endpoint,
        session->authkey);
  } else {
    req_size = snprintf(NULL, 0, HTTP_GET_REQUEST_FMT, path, session->endpoint);
    request = calloc(((unsigned long)req_size + 1), sizeof(char));
    sprintf(request, HTTP_GET_REQUEST_FMT, path, session->endpoint);
  }

  if (req_size != SSL_write(session->ssl, request, req_size)) {
    pprint_error("%s@%s:%d couldn't write to socket (aborting)", __FILE_NAME__,
        __func__, __LINE__);
    abort();
  }

  free(request);

  char *response_ = NULL;
  size_t response_len = 0;

  _http_ssl_read_all(session->ssl, &response_, &response_len);
  *response = response_;
}

char *
http_get_request_generate(struct httpwss_session *session, char *path)
{
  char *request;
  int req_size;
  if (session->hashauth) {
    req_size = snprintf(NULL, 0, HTTP_GET_REQUEST_AUTH_FMT, path,
        session->endpoint, session->authkey);
    request = calloc(((unsigned long)req_size + 1), sizeof(char));
    sprintf(request, HTTP_GET_REQUEST_AUTH_FMT, path, session->endpoint,
        session->authkey);
  } else {
    req_size = snprintf(NULL, 0, HTTP_GET_REQUEST_FMT, path, session->endpoint);
    request = calloc(((unsigned long)req_size + 1), sizeof(char));
    sprintf(request, HTTP_GET_REQUEST_FMT, path, session->endpoint);
  }

  return request;
}

void
http_get_request_cached(struct httpwss_session *session, char *request,
    int req_size, char **response, char *record)
{
  if (req_size != SSL_write(session->ssl, request, req_size)) {
    pprint_error("%s@%s:%d couldn't write to socket (aborting)", __FILE_NAME__,
        __func__, __LINE__);
    abort();
  }

  char *_local_response = NULL;
  size_t _local_len = 0;
  (void) record;

  _http_ssl_read_all(session->ssl, &_local_response, &_local_len);
  *response = _local_response;
}

struct httpwss_session *
httpwss_session_new(char *endpoint, char *port)
{

  struct httpwss_session *session = calloc(1, sizeof(struct httpwss_session));

  if (!session) {
    pprint_error("%s@%s:%d no more memory (aborting)", __FILE_NAME__, __func__,
        __LINE__);
    abort();
  }

  session->endpoint = strdup(endpoint);
  session->iswss = false;

  // convert endpoint to an ip address
  struct addrinfo *res = NULL;

  if (getaddrinfo(endpoint, port, NULL, &res) != 0) {
    pprint_error("%s@%s:%d unable to resolve %s (aborting)", __FILE_NAME__,
        __func__, __LINE__, endpoint);
    abort();
  }

  // create the socket
  session->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (session->fd == -1) {
    pprint_error("%s@%s:%d socket fd failed (aborting)", __FILE_NAME__,
        __func__, __LINE__);
    pprint_error("socket returned %d", session->fd);
    abort();
  }

  pprint_info("connection requested...");
  // connect the socket to the remote host
  connect(session->fd, res->ai_addr, res->ai_addrlen);
  if (errno != 115) {
    pprint_error("%s@%s:%d connection failed for %s on port %s (aborting)",
        __FILE_NAME__, __func__, __LINE__, endpoint, port);
    pprint_error("errrno[%d]: %s", errno, strerror(errno));
    abort();
  }

  // poll the socket until read
  struct pollfd pfd;
  pfd.fd = session->fd;
  pfd.events = POLLOUT | POLLIN;

  poll(&pfd, 1, -1);
  pprint_info("connection established");

  // prime SSL for establishing TLS connection
  const SSL_METHOD *method = TLS_client_method();
  SSL_CTX *ctx = SSL_CTX_new(method);

  if (ctx == NULL) {
    ERR_print_errors_fp(stdout);
    abort();
  }

  session->ssl = SSL_new(ctx);
  SSL_set_fd(session->ssl, session->fd);
  SSL_set_tlsext_host_name(session->ssl, endpoint);

  // perform the TLS handshake
  int ret = 0;
  while ((ret = SSL_connect(session->ssl)) && ret <= 0) {
    switch (SSL_get_error(session->ssl, ret)) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      poll(&pfd, 1, -1);
      break;
    default:
      abort();
    }
  }

  pprint_info("ssl context established");

  SSL_CTX_free(ctx);

  freeaddrinfo(res);

  session->authkey = NULL;
  session->hashauth = false;

  return session;
}

void
httpwss_session_free(struct httpwss_session *session)
{
  free(session->endpoint);
  SSL_free(session->ssl);
  free(session);
}
