#ifndef SESSION_H
#define SESSION_H

/*
 * OpenSSL library
 */
#include <openssl/ssl.h>

/*
 * c sockets
 */
#include <sys/socket.h>

/*
 * pretty printing
 */
#include <pprint/pprint.h>

/*
 * global status returns
 */
#include <status.h>

/*
 * gethostbyname
 */
#include <netdb.h>

/*
 * exports errorno
 */
#include <errno.h>

/*
 * The current length of the pre encoded key needed for web socket
 * communication.
 */
#define HTTP_WSS_KEY_LEN 16

/*
 * Structure that represents a session towards a websocket connection.
 */
struct tls_session {
  /*
   * raw file descriptor for socket
   */
  int fd;

  /*
   * an SSL instance for SSL/TLS communication
   */
  SSL *ssl;

  /*
   * the location this session is remoted into to
   */
  char *endpoint;

  /*
   * the port of this active session
   */
  char *port;

};

/*
 * Creates a new tls session to be used.
 *
 * @param endpoint the endpoing to connecting to
 * @param port the port to connect to
 * @param _session a place to allocate and store the new session
 * @return a status code for failure/success
 */
status_t tls_session_new(char *endpoint, char *port,
    struct tls_session **_session);

/*
 * Cleans up the httpwss session that was created using the httpwss_session_new
 * function.
 *
 * @param session a reference to the session pointer created by
 * httpwss_session_new
 */
status_t tls_session_free(struct tls_session **session);

#endif
