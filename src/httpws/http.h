#ifndef HTTP_H
#define HTTP_H

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
 * Upgrades an HTTP session to web socket. This is done by sending an HTTP
 * upgrade request. This function will send the request and verify the
 * response.
 *
 * @param ssl an active SSL context
 * @param path the path to request to normally / but can be /chat etc..
 * @return Returns 0 on success and 1 if wss_upgrade was no successful
 */
int http_wss_upgrade(SSL *ssl, char *endpoint, char *path);

/*
 * Performs an HTTP get request and kills the connection this conenction
 * will NOT be kept alive. This function assumes an SSL/TLS connection
 * and will fail if used on non secure tunnel.
 * @param url the url to fetch
 * @param path the endpoint to perform the get request
 * @param res pointer to storage of result
 * @return Returns 0 on success and 1 if failure.
 */
int http_get_request(char *url, char *path, char **res);

#endif
