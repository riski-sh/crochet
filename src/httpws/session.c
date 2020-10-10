#include "session.h"

status_t
tls_session_new(char *endpoint, char *port, struct tls_session **_session)
{

  if (*_session != NULL) {
    pprint_error("%s", "*_session != NULL, refusing to reuse address");
    return STATUS_EXPECTED_NULL;
  }

  /*
   * Create a structure to hold the session
   */
  (*_session) = malloc(sizeof(struct tls_session));
  PPRINT_CHECK_ALLOC((*_session));

  /*
   * Copy over the endpoint in case of reconnect
   */
  (*_session)->endpoint = strdup(endpoint);
  PPRINT_CHECK_ALLOC((*_session)->endpoint);

  /*
   * Copy over the port in case of reconnect
   */
  (*_session)->port = strdup(port);
  PPRINT_CHECK_ALLOC((*_session)->port);

  pprint_info("resolving to %s:%s...", endpoint, port);

  /*
   * Get the IP address information of the host
   */
  struct addrinfo *info = NULL;

  /*
   * Set hints to only query ipv4 addresses
   */
  int ret = getaddrinfo(endpoint, port, NULL, &info);

  /*
   * Create a socket to communicate on
   */
  pprint_info("%s", "establishing socket...");
  (*_session)->fd = socket(info->ai_family, SOCK_STREAM, 0);
  if ((*_session)->fd <= 0) {
    pprint_error("%s", strerror(errno));
    pprint_error("%s", "unable to obtain socket");
    return STATUS_UNKNOWN_ERROR;
  }
  pprint_info("obtained socked %d", (*_session)->fd);

  /*
   * Connect to the remote host
   */
  pprint_info("%s", "connecting to remote host...");
  ret = connect((*_session)->fd, info->ai_addr, info->ai_addrlen);
  if (ret != 0) {
    pprint_error("unable to connect to %s", endpoint);
    pprint_error("%s", strerror(errno));
    return STATUS_UNKNOWN_ERROR;
  }
  pprint_info("%s", "connection succesfull");

  /*
   * Establish TLS connection
   */
  const SSL_METHOD *method = TLS_client_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  (*_session)->ssl = SSL_new(ctx);

  /*
   * Make sure we pin ourselves to the certificate belonging to the hostname
   */
  SSL_set_fd((*_session)->ssl, (*_session)->fd);
  SSL_set_tlsext_host_name((*_session)->ssl, endpoint);

  pprint_info("%s", "establishing TLS handshake...");
  ret = SSL_connect((*_session)->ssl);
  pprint_info("%s", "TLS handshake valid");

  /*
   * Free the context as it is no longer needed as well as the address info
   * since we have already connected to the remote host
   */
  SSL_CTX_free(ctx);
  freeaddrinfo(info);

  return STATUS_OK;
}

status_t
tls_session_reconnect(struct tls_session *session)
{
  SSL_free(session->ssl);
  close(session->fd);

  /*
   * Get the IP address information of the host
   */
  struct addrinfo *info = NULL;

  /*
   * Set hints to only query ipv4 addresses
   */
  int ret = getaddrinfo(session->endpoint, session->port, NULL, &info);

  /*
   * Create a socket to communicate on
   */
  pprint_info("%s", "establishing socket...");
  session->fd = socket(info->ai_family, SOCK_STREAM, 0);
  if (session->fd <= 0) {
    pprint_error("%s", strerror(errno));
    pprint_error("%s", "unable to obtain socket");
    return STATUS_UNKNOWN_ERROR;
  }
  pprint_info("obtained socked %d", session->fd);

  /*
   * Connect to the remote host
   */
  pprint_info("%s", "connecting to remote host...");
  ret = connect(session->fd, info->ai_addr, info->ai_addrlen);
  if (ret != 0) {
    pprint_error("unable to connect to %s", session->endpoint);
    pprint_error("%s", strerror(errno));
    return STATUS_UNKNOWN_ERROR;
  }
  pprint_info("%s", "connection succesfull");

  /*
   * Establish TLS connection
   */
  const SSL_METHOD *method = TLS_client_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  session->ssl = SSL_new(ctx);

  /*
   * Make sure we pin ourselves to the certificate belonging to the hostname
   */
  SSL_set_fd(session->ssl, session->fd);
  SSL_set_tlsext_host_name(session->ssl, session->endpoint);

  pprint_info("%s", "establishing TLS handshake...");
  ret = SSL_connect(session->ssl);
  pprint_info("%s", "TLS handshake valid");

  /*
   * Free the context as it is no longer needed as well as the address info
   * since we have already connected to the remote host
   */
  SSL_CTX_free(ctx);
  freeaddrinfo(info);


  return STATUS_OK;
}

status_t
tls_session_free(struct tls_session **session)
{
  SSL_free((*session)->ssl);
  free((*session)->endpoint);
  free((*session)->port);
  free(*session);
  *session = NULL;

  return STATUS_OK;
}
