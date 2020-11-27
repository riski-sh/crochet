#ifndef WEB_COMMS_H
#define WEB_COMMS_H

#include <stdlib.h>
#include <ffjson/ffjson.h>
#include <pprint/pprint.h>

/*
 * Parses the incoming message and allocates _response and sets the return
 * message in that _response
 * @param msg the message
 */
void comms_parse_message(char *msg, char **_response);

#endif
