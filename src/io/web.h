#ifndef WEB_H
#define WEB_H

#include <pprint/pprint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Initalizes communication with webserver by creating a named pipe for golang
 * to communicate and then starts the golang web server
 */
void web_init();

#endif
