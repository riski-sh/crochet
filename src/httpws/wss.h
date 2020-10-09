/*
 * The web socket portion of the http web socket library used in crochet is a
 * fine tuned implementation to allow for the fastest possible communication
 * between client and server. The websocket client at all costs to avoid
 * calling *alloc functions. Since crochet manages multiple connections to
 * exchanges, every clock cycle counts and malloc is a difficult function
 * to predict clock cycles with since requesting memory can take anywhere
 * between 1 - 400+ clock cycles. Because of the nature of this program and
 * the unpredictable nature of malloc clock cycles, code readability suffers
 * in order to prioritize stack memory over heap.
 */

#ifndef WSS_H
#define WSS_H

#include <arpa/inet.h>
#include <limits.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pprint/pprint.h>
#include <unistd.h>

#include "session.h"

enum WSS_ERR {
  WSS_ERR_NONE = 0,
  WSS_ERR_GET_ADDR_INFO = 1,
  WSS_ERR_SOCKET_CREATION = 2,
  WSS_ERR_CONNECT_FAILURE = 3,

  // number of web socket error codes
  WSS_ERR_NUM
};

/*
 * Established a remote connection and upgrade the request from HTTP to web
 * socket. This only establishes secure from HTTP to web socket. This only
 * establishes secure TLS connections to the remote server.
 *
 * @param endpoint the host to connect to
 * @param port the port to connect, this will almost always be 443
 * @param _session a structure owned by the caller that will get populated with
 * the appropriate values to read/write to the web socket connection.
 */
enum WSS_ERR wss_client(
    char *endpoint, char *path, char *port, struct tls_session **_session);

enum WSS_ERR wss_send_text(
    struct tls_session *session, unsigned char *text, size_t len);

enum WSS_ERR wss_read_text(struct tls_session *session, char **value);

#endif
