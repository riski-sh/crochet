#include <netdb.h>
#include <openssl/ssl.h>
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

#define HTTP_GET_REQUEST_AUTH_FMT      \
  "GET %s HTTP/1.1\r\n"                \
  "Host: %s\r\n"                       \
  "User-Agent: crochet\r\n"            \
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

static struct _http_response *_http_parse_response(char **res);

/*
 * Read everything into one buffer
 */
static void
_http_ssl_read_all(SSL *ssl, char **_r, uint32_t *_n)
{
  // SSL fragment can contain maximum of 16kb

  int total_read = 0;
  char *res = NULL;
  char *root_res_ptr = NULL;
  int fragment_size = 0;

  char record[16384] = {'\x0'};
  while (fragment_size == 0) {
    fragment_size = SSL_read(ssl, record, 16383);
    if (fragment_size == 0) {
      *_r = NULL;
      *_n = 0;
      return;
    }
    res = calloc((size_t)(total_read + fragment_size + 1), sizeof(char));
    root_res_ptr = res;
    memcpy(&(res[total_read]), record, (size_t)fragment_size);
    total_read += fragment_size;
  }

  struct _http_response *headers = _http_parse_response(&res);
  data_t read_format = UNKNOWN;
  while (headers) {
    if (headers->header_name) {
      if (strcmp(headers->header_name, "Content-Length") == 0) {
        read_format = CONTENT_LENGTH;
        break;
      } else if (strcmp(headers->header_name, "Transfer-Encoding") == 0) {
        read_format = CHUNCKED;
        break;
      }
    }
    headers = headers->next;
  }

  if (read_format == CONTENT_LENGTH) {
    // data remaining
    int data_remaining = (int)strlen(res);
    int content_length = atoi(headers->header_value);

    if (data_remaining != content_length) {
      abort();
    }

    // entire data length is contained inside this record
    if (data_remaining == content_length) {
      *_r = strndup(res, (size_t)content_length);
      *_n = (uint32_t)content_length;
      free(root_res_ptr);
    }
  } else if (read_format == CHUNCKED) {
    int data_remaining = (int)strlen(res);
    if (data_remaining > 0) {
      char *chunk_save = NULL;
      char *first_chunk_size_str = strtok_r(res, "\n", &chunk_save);
      char *nxt_data = strtok_r(chunk_save, "\r", &chunk_save);

      size_t total_chunk_size =
          strtoul(first_chunk_size_str, NULL, 16);

      char *chunk = calloc(total_chunk_size + 1, sizeof(char));
      if (!chunk) {
        printf("error can't create memory?\n");
        abort();
      }
      size_t global_total_read = total_chunk_size;

      size_t partial_chunk_len = strlen(nxt_data);
      memcpy(chunk, nxt_data, partial_chunk_len);

      size_t chunk_read = 0;
      char endings[2];

      if (partial_chunk_len != total_chunk_size) {
        chunk_read = partial_chunk_len;
        while (chunk_read != total_chunk_size) {
          chunk_read += (size_t) SSL_read(ssl, &(chunk[chunk_read]), (int)(total_chunk_size-chunk_read));
        }
        SSL_read(ssl, endings, 2);
      }

      free(root_res_ptr);

      while (1) {
        char chunk_len[9] = {0};
        int chunk_len_idx = 0;
        do {
          // loop until \r
          if (chunk_len_idx >= 8) {
            exit(1);
          }
          chunk_len_idx += SSL_read(ssl, &(chunk_len[chunk_len_idx]), 2);
          if (chunk_len[chunk_len_idx-1] == '\r') {
            SSL_read(ssl, endings, 1);
            chunk_len[chunk_len_idx-1] = '\x0';
            break;
          } else if (chunk_len[chunk_len_idx-1] == '\n') {
            chunk_len[chunk_len_idx-2] = '\x0';
            break;
          }
        } while (1);
        total_chunk_size = strtoul(chunk_len, NULL, 16);

        if (total_chunk_size == 0) {
          SSL_read(ssl, endings, 2);
          break;
        }
        chunk_read = 0;
        chunk = realloc(chunk, global_total_read + total_chunk_size + 1);
        if (!chunk) {
          abort();
        }

        while (chunk_read != total_chunk_size) {
          int ret = SSL_read(ssl, &(chunk[global_total_read]), (int)(total_chunk_size - chunk_read));
          global_total_read += (size_t)ret;
          chunk_read += (size_t)ret;
          chunk[global_total_read] = '\x0';
        }
        SSL_read(ssl, endings, 2);
      }
      chunk[global_total_read] = '\x0';
      *_r = chunk;
      *_n = (uint32_t) global_total_read;
      return;
    }
  } else {
    abort();
  }
}

static void
_http_response_free(struct _http_response *res)
{
  while (res) {
    struct _http_response *cur = res;
    res = res->next;
    free(cur);
  }
}

static struct _http_response *
_http_parse_response(char **res)
{

  struct _http_response *response = malloc(sizeof(struct _http_response));

  // the first token
  char *root_save = NULL;
  char *tok = strtok_r(*res, "\n", &root_save);
  tok = strtok_r(root_save, "\n", &root_save);

  struct _http_response *cur = response;
  cur->header_name = NULL;
  cur->header_value = NULL;

  while (tok[0] != '\r') {
    cur->next = malloc(sizeof(struct _http_response));
    cur = cur->next;

    char *header_save = NULL;
    char *name = strtok_r(tok, ":", &header_save);
    char *value = strtok_r(header_save, ":", &header_save);

    cur->header_name = name;
    cur->header_value = value;
    cur->next = NULL;

    tok = strtok_r(root_save, "\n", &root_save);
  }

  cur->next = NULL;

  *res = root_save;
  return response;
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

  free(request);

  // get the response headers with values and find the
  // Sec-WebSocket-Accept header.

  char *response = NULL;
  uint32_t test = 0;

  _http_ssl_read_all(session->ssl, &response, &test);

  struct _http_response *responses = _http_parse_response(&response);
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

  char *_local_response = NULL;
  uint32_t _local_len = 0;

  if (req_size != SSL_write(session->ssl, request, req_size)) {
      printf("didn't write everything\n");
      abort();
  }

  free(request);

  _http_ssl_read_all(session->ssl, &_local_response, &_local_len);
  *response = _local_response;

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
