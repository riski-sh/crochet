#include <netdb.h>
#include <openssl/ssl.h>
#include <stdlib.h>
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

#define HTTP_GET_REQUEST_AUTH_FMT      \
  "GET %s HTTP/1.1\r\n"                \
  "Host: %s\r\n"                       \
  "Connection: keep-alive\r\n"         \
  "User-Agent: crochet\r\n"            \
  "Content-Type: application/json\r\n" \
  "Accept-Encoding: deflate\r\n"       \
  "Accept-Datetime-Format: UNIX\r\n"   \
  "Authorization: Bearer %s\r\n"       \
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

static void
_http_response_free(struct _http_response *res)
{
  while (res) {
    free(res->header_name);
    free(res->header_value);
    struct _http_response *cur = res;
    res = res->next;
    free(cur);
  }
}

/*
 * A private function that reads a line of the http headers response
 * @param ssl the ssl context to read from
 * @param buf the buffer to put the data in
 */
static bool
_http_ssl_getline(SSL *ssl, char buf[4096], size_t *line_length)
{
  size_t buf_len = 0;

  for (; buf_len < 4096; ++buf_len) {
    int ret = 0;
    do {
      ret = SSL_read(ssl, &(buf[buf_len]), 1);
    } while (ret < 0 );
    if (buf[buf_len] == '\n') {
      buf[buf_len] = '\x0';
      break;
    }
  }

  if (buf_len == 4096) {
    pprint_error("haulting on buffer overflow", __FILE_NAME__, __func__,
        __LINE__);
    exit(1);
  }

  *line_length = buf_len;
  return buf_len > 1;
}

static struct _http_response *
_http_parse_response(SSL *ssl)
{
  char line[4096] = {0};
  size_t line_len = 0;
  _http_ssl_getline(ssl, line, &line_len);

  struct _http_response *response = malloc(sizeof(struct _http_response));

  response->header_name = strdup(line);
  response->header_value = NULL;
  response->next = NULL;

  struct _http_response *cur = response;

  while ((_http_ssl_getline(ssl, line, &line_len))) {
    cur->next = malloc(sizeof(struct _http_response));
    cur = cur->next;

    char *name = NULL;
    char *value = NULL;

    size_t col_idx = 0;
    for (; col_idx < line_len; ++col_idx) {
      if (line[col_idx] == ':') {
        break;
      }
    }
    if (col_idx == line_len) {
      pprint_error(
          "invalid http response %s", __FILE_NAME__, __func__, __LINE__, line);
      abort();
    }

    name = malloc(sizeof(char) * (col_idx + 1));
    if (!name) {
      pprint_error("no more memory", __FILE_NAME__, __func__, __LINE__);
      abort();
    }

    for (size_t i = 0; i < col_idx; ++i) {
      name[i] = line[i];
    }
    name[col_idx] = '\x0';

    value = malloc(sizeof(char) * line_len);
    if (!value) {
      pprint_error("no more memory", __FILE_NAME__, __func__, __LINE__);
      abort();
    }

    for (size_t i = (col_idx + 2); i < line_len; ++i) {
      value[i - (col_idx + 2)] = line[i];
    }
    value[line_len - col_idx - 2] = '\x0';

    cur->header_name = strdup(name);
    cur->header_value = strdup(value);
    cur->next = NULL;
  }

  cur->next = NULL;

  return response;
}

static char *
_http_read_chuncked(SSL *ssl)
{
  // read the size line and convert the hex number to
  // decimal

  size_t total_size = 0;
  char *data = NULL;

  char _len[4096] = {0};
  size_t __len = 0;

  while (true) {

    _http_ssl_getline(ssl, _len, &__len);

    if (__len == 1) {
      continue;
    }

    size_t len = (size_t)strtol(_len, NULL, 16);
    if (len == 0) {
      break;
    }

    data = realloc(data, total_size + len + 1);
    size_t read = 0;
    while (read != len) {
      read +=
          (size_t)SSL_read(ssl, &(data[total_size + read]), (int)(len - read));
    }
    total_size += len;
    data[total_size] = '\x0';
  }

  _http_ssl_getline(ssl, _len, &__len);

  return data;
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
      __FILE_NAME__, __func__, __LINE__, raw_key[0], raw_key[1], raw_key[2],
      raw_key[3], raw_key[4], raw_key[5], raw_key[6], raw_key[7], raw_key[8],
      raw_key[9], raw_key[10], raw_key[11], raw_key[12], raw_key[13],
      raw_key[14], raw_key[15]);

  unsigned char *key_encoded = NULL;
  key_encoded = base64(raw_key, HTTP_WSS_KEY_LEN);

  pprint_info("generated Sec-WebSocket-Key: %s", __FILE_NAME__, __func__,
      __LINE__, key_encoded);

  int req_size = snprintf(
      NULL, 0, HTTP_WSS_UPGRADE_FMT, path, session->endpoint, key_encoded);

  char *request = (char *)malloc(((unsigned long)req_size + 1) * sizeof(char));
  sprintf(request, HTTP_WSS_UPGRADE_FMT, path, session->endpoint, key_encoded);

  // don't send over the NULL terminator
  SSL_write(session->ssl, request, req_size);

  // get the response headers with values and find the
  // Sec-WebSocket-Accept header.
  struct _http_response *responses = _http_parse_response(session->ssl);
  struct _http_response *iter = responses;
  while (strcmp(iter->header_name, "Sec-WebSocket-Accept") != 0) {
    iter = iter->next;
  }

  if (iter) {
    pprint_info("confirmation key %s received web socket upgrade complete ",
        __FILE_NAME__, __func__, __LINE__, iter->header_value);
  } else {
    pprint_error("server did not accept websocket upgrade", __FILE_NAME__,
        __func__, __LINE__);
    abort();
  }

  free(key_encoded);
  _http_response_free(responses);

  session->iswss = true;
  return 0;
}

int
http_get_request(struct httpwss_session *session, char *path, char **response)
{

  int req_size;
  char *request;

  if (session->hashauth) {
    req_size = snprintf(NULL, 0, HTTP_GET_REQUEST_AUTH_FMT, path,
        session->endpoint, session->authkey);
    request = (char *)malloc(((unsigned long)req_size + 1) * sizeof(char));
    sprintf(request, HTTP_GET_REQUEST_AUTH_FMT, path, session->endpoint,
        session->authkey);
  } else {
    req_size = snprintf(NULL, 0, HTTP_GET_REQUEST_FMT, path, session->endpoint);
    request = (char *)malloc(((unsigned long)req_size + 1) * sizeof(char));
    sprintf(request, HTTP_GET_REQUEST_FMT, path, session->endpoint);
  }

  SSL_write(session->ssl, request, req_size);

  free(request);

  struct _http_response *responses = _http_parse_response(session->ssl);
  struct _http_response *iter = responses;

  data_t read_format = UNKNOWN;
  while (iter) {
    if (iter->header_name) {
      if (strcmp(iter->header_name, "Content-Length") == 0) {
        read_format = CONTENT_LENGTH;
        break;
      } else if (strcmp(iter->header_name, "Transfer-Encoding") == 0) {
        read_format = CHUNCKED;
        break;
      }
    }
    iter = iter->next;
  }

  if (read_format == UNKNOWN) {
    pprint_info("i don't know how to read this response", __FILE_NAME__,
        __func__, __LINE__);
    exit(1);
  }

  char *body = NULL;

  if (read_format == CONTENT_LENGTH) {
    int content_length = atoi(iter->header_value);

    body = (char *)malloc((size_t)(content_length + 1) * sizeof(char));
    int num_read = 0;
    while (num_read != content_length) {
      num_read += SSL_read(
          session->ssl, &(body[num_read]), (content_length - num_read));
    }
    body[content_length] = '\x0';

  } else if (read_format == CHUNCKED) {
    body = _http_read_chuncked(session->ssl);
  }

  _http_response_free(responses);
  *response = body;
  return 0;
}

struct httpwss_session *
httpwss_session_new(char *endpoint, char *port)
{

  struct httpwss_session *session = malloc(sizeof(struct httpwss_session));

  session->endpoint = strdup(endpoint);
  session->iswss = false;

  if (!session) {
    pprint_error("no more memory", __FILE_NAME__, __func__, __LINE__);
    exit(1);
  }

  pprint_info("starting connection to %s:%s", __FILE_NAME__, __func__, __LINE__,
      endpoint, port);

  // convert endpoint to an ip address
  struct addrinfo *res = NULL;

  if (getaddrinfo(endpoint, port, NULL, &res) != 0) {
    pprint_error(
        "unable to resolve %s", __FILE_NAME__, __func__, __LINE__, endpoint);
    // return WSS_ERR_GET_ADDR_INFO;
  }

  // create the socket
  session->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (session->fd == -1) {
    pprint_error("unable to create socket probably because one is already open",
        __FILE_NAME__, __func__, __LINE__);
    // return WSS_ERR_SOCKET_CREATION;
  }

  // connect the socket to the remote host
  if (connect(session->fd, res->ai_addr, res->ai_addrlen) == -1) {
    // return WSS_ERR_CONNECT_FAILURE;
  }

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
  if (SSL_connect(session->ssl) == -1) {
    ERR_print_errors_fp(stdout);
    abort();
  }

  SSL_CTX_free(ctx);

  pprint_info("tls handshake accepted by %s", __FILE_NAME__, __func__, __LINE__,
      endpoint);

  freeaddrinfo(res);

  session->authkey = NULL;
  session->hashauth = false;

  return session;
}

void
httpwss_session_free(struct httpwss_session *session)
{
  if (session->hashauth) {
    free(session->authkey);
  }
  free(session->endpoint);
  SSL_free(session->ssl);
  free(session);
}
