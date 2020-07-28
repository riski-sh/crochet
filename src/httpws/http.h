#ifndef HTTP_H
#define HTTP_H

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

#endif
