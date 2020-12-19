#ifndef WEB_H
#define WEB_H

#include <globals/globals.h>
#include <libwebsockets.h>
#include <signal.h>
#include <string.h>
#include <globals/globals.h>
#include <web/comms.h>

/*
 * Starts the main, websocket server
 *
 * @param argc argc variable passed through from the main function
 * @param argv argv variable passed through from the main function
 * @param cert the file location of the certificate
 * @param key the file location of the private key file
 * @param port the port to listen to must be greater than 1000
 *
 * @return Returns true when shutdown was graceful, false if error has occured
 */
bool
server_start(int argc, const char **argv, char *cert, char *key, int port);

#endif
