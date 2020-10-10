#include "server.h"

static int
_create_socket(int port)
{
  int s;
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (s < 0) {
    perror("Unable to create socket");
    exit(EXIT_FAILURE);
  }

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Unable to bind");
    exit(EXIT_FAILURE);
  }

  if (listen(s, 1) < 0) {
    perror("Unable to listen");
    exit(EXIT_FAILURE);
  }

  return s;
}

static void
_init_openssl()
{
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

static void
_cleanup_openssl()
{
  EVP_cleanup();
}

static SSL_CTX *
_create_context()
{
  const SSL_METHOD *method;
  SSL_CTX *ctx;

  method = TLS_server_method();

  ctx = SSL_CTX_new(method);
  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  return ctx;
}

static void
_configure_context(SSL_CTX *ctx, char *cert, char *key)
{
  SSL_CTX_set_ecdh_auto(ctx, 1);

  /* Set the key and cert */
  if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
}

void *
server_serv(void *cfg)
{

  struct server_config *config = cfg;

  int sock;
  SSL_CTX *ctx;

  _init_openssl();
  ctx = _create_context();

  _configure_context(ctx, config->cert, config->key);

  sock = _create_socket(4433);

  struct pollfd pfds;
  pfds.fd = sock;
  pfds.events = POLLIN;

  /* Handle connections */
  while (globals_continue(NULL)) {
    struct sockaddr_in addr;
    uint len = sizeof(addr);
    SSL *ssl;
    const char reply[] = "test\n";

    int ret = poll(&pfds, (unsigned int)1, 1);

    switch (ret) {
    case 0:
      // timeout expired
      break;
    case -1:
      perror("Error on poll");
      break;
    default: {
      int client = accept(sock, (struct sockaddr *)&addr, &len);
      pprint_info("accept connection %d", client);
      if (client < 0) {
        perror("Unable to accept");
        exit(EXIT_FAILURE);
      }

      ssl = SSL_new(ctx);
      SSL_set_fd(ssl, client);

      if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
      } else {
        SSL_write(ssl, reply, strlen(reply));
      }

      SSL_shutdown(ssl);
      SSL_free(ssl);
      close(client);
    }
    }
  }

  close(sock);
  SSL_CTX_free(ctx);
  _cleanup_openssl();
  return NULL;
}
