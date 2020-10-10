#ifndef SERVER_H
#define SERVER_H

#include <sys/poll.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <globals/globals.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pprint/pprint.h>
#include <status.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct server_config {
  char *cert;
  char *key;
};

void *server_serv(void *cfg);

#endif
