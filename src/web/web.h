#ifndef WEB_H
#define WEB_H

#include <globals/globals.h>
#include <libwebsockets.h>
#include <signal.h>
#include <string.h>

int
server_start(int argc, const char **argv);

#endif
