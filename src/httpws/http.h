#ifndef HTTP_H
#define HTTP_H

#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pprint.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "base64.h"

#define HTTP_CR_LF "\r\n"

/*
 * The current length of the pre encoded key needed for web socket
 * communication.
 */
#define HTTP_WSS_KEY_LEN 16

/*
 * Structure that represents a session towards a websocket connection.
 */
struct httpwss_session {
  /*
   * raw file descriptor for socket
   */
  int fd;

  /*
   * struct padding
   */
  char _p1[4];

  /*
   * the host this session is connected to
   */
  char *endpoint;

  /*
   * an SSL instance for SSL/TLS communication
   */
  SSL *ssl;

  char _p2[7];

  /*
   * set to true if this session has been upgraded to a websocket
   */
  bool iswss;

  /*
   * send an authorization header
   */
  bool hashauth;

  char _p3[7];

  /*
   * the authorization key if needed
   */
  char *authkey;
};

/*
 * Upgrades an HTTP session to web socket. This is done by sending an HTTP
 * upgrade request. This function will send the request and verify the
 * response.
 *
 * @param session the session to upgrade
 * @param path the path to request to normally / but can be /chat etc..
 * @return Returns 0 on success and 1 if wss_upgrade was no successful
 */
int http_wss_upgrade(struct httpwss_session *session, char *path);

/*
 * Performs an HTTP get request
 * This function assumes an SSL/TLS connection
 * and will fail if used on non secure tunnel.
 *
 * @param session the session to perform this request on
 * @param path the endpoint to perform the get request
 * @param res pointer to storage of result
 * @return Returns 0 on success and 1 if failure.
 */
int http_get_request(struct httpwss_session *session, char *path, char **res);

struct httpwss_session *httpwss_session_new(char *endpoint, char *port);

void httpwss_session_free(struct httpwss_session *session);

#endif
