#ifndef WEB_COMMS_H
#define WEB_COMMS_H

#include <stdlib.h>
#include <ffjson/ffjson.h>
#include <pprint/pprint.h>
#include <exchanges/exhangesall.h>
#include <security/security.h>
#include <string/string.h>
#include <libwebsockets.h>

/*
 * Parses the incoming message and allocates _response and sets the return
 * message in that _response
 * @param msg the message
 * @param _response the resposne to allocate and send to the client
 * @param _len the length of the message
 */
void comms_parse_message(char *msg, char **_response, size_t *_len);

#endif
