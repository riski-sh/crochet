#ifndef WEB_H
#define WEB_H

#include <globals/globals.h>
#include <libwebsockets.h>
#include <signal.h>
#include <string.h>
#include <globals/globals.h>
#include <web/comms.h>

int
server_start(int argc, const char **argv, char *cert, char *key);

#endif
