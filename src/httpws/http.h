#ifndef HTTP_H
#define HTTP_H

#include <sys/select.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pprint.h>
#include <stdbool.h>

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
 */
void http_get_request(struct httpwss_session *session, char *path, char **res);

/*
 * Generates a the request header for performing an http get request.
 * This can be useful if you have to keep on repeating the same request.
 * The output of http_get_request_generate is ment to be passed along
 * with http_get_request_cached which will not regenerate the get request
 * and avoid expensive calls to snprintf.
 *
 * @param session the session that this request will be used on
 * @param path the path of the resource needed to be fetched.
 */
char *http_get_request_generate(struct httpwss_session *session, char *path);

/*
 * Uses a pre defined request to perform the http_get_request.
 *
 * @param session the session to perform this on get request on
 * @param request the request to send
 * @param req_size the length of the request
 * @param response a place to store the response string
 * @param record a cached record value to be used -- note it is recommended to
 * use a record size of 16384.
 *
 */
void http_get_request_cached(struct httpwss_session *session, char *request,
    int req_size, char **response, char *record);

/*
 * Creates a new httpwss session to be used.
 *
 * @param endpoint the endpoing to connecting to
 * @param port the port to connect to
 */
struct httpwss_session *httpwss_session_new(char *endpoint, char *port);

/*
 * Cleans up the httpwss session that was created using the httpwss_session_new
 * function
 */
void httpwss_session_free(struct httpwss_session *session);

#endif
