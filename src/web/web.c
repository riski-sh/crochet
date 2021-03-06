/*
 * lws-minimal-ws-server
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This demonstrates the most minimal http server you can make with lws,
 * with an added websocket chat server.
 *
 * To keep it simple, it serves stuff in the subdirectory "./mount-origin" of
 * the directory it was started in.
 * You can change that by changing mount.origin.
 */

#include "web.h"

#define LWS_PLUGIN_STATIC
#include "protocol_lws_minimal.c"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
static struct lws_protocols protocols[] = {
    {"http", lws_callback_http_dummy, 0, 0},
    LWS_PLUGIN_PROTOCOL_MINIMAL,
    {NULL, NULL, 0, 0} /* terminator */
};

static const lws_retry_bo_t retry = {
    .secs_since_valid_ping = 3,
    .secs_since_valid_hangup = 10,
};

static const struct lws_protocol_vhost_options archive_mimes = {
  NULL,
  NULL,
  ".csv",
  "text/csv"
};

static const struct lws_http_mount mount_archive = {
    /* .mount_next */ NULL,
    /* .mountpoint */ "/archive",
    /* .origin */ "/opt/riski-sh/archive",
    /* .def */ NULL,
    /* .protocol */ NULL,
    /* cgienv */ NULL,
    /* .extra_mimetypes */ &archive_mimes,
    /* .interpret */ NULL,
    /* .cgi_timeout */ 0,
    /* .cache_max_age */ 0,
    /* auth_mask */ 0,
    /* .cache_reusable */ 0,
    /* cache_revalidate */ 0,
    /* .cache_intermediaries */ 0,
    /* .origin_protocol */ LWSMPRO_FILE,
    /* .mountpoint_len */ 8,
    /* .basic_auth_login_file */ NULL,
};

static const struct lws_http_mount mount = {
    /* .mount_next */ &mount_archive,   /* linked-list "next" */
    /* .mountpoint */ "/",              /* mountpoint URL */
    /* .origin */ "/opt/riski-sh/web/", /* serve from dir */
    /* .def */ "index.html",            /* default filename */
    /* .protocol */ NULL,
    /* .cgienv */ NULL,
    /* .extra_mimetypes */ NULL,
    /* .interpret */ NULL,
    /* .cgi_timeout */ 0,
    /* .cache_max_age */ 0,
    /* .auth_mask */ 0,
    /* .cache_reusable */ 0,
    /* .cache_revalidate */ 0,
    /* .cache_intermediaries */ 0,
    /* .origin_protocol */ LWSMPRO_FILE, /* files in a dir */
    /* .mountpoint_len */ 1,             /* char count */
    /* .basic_auth_login_file */ NULL,
};
#pragma clang diagnostic pop

bool
server_start(int argc, const char **argv, char *cert, char *key, int port)
{
  struct lws_context_creation_info info;
  struct lws_context *context;
  const char *p;
  int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
      /* for LLL_ verbosity above NOTICE to be built into lws,
       * lws must have been configured and built with
       * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
      /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
      /* | LLL_EXT */ /* | LLL_CLIENT */  /* | LLL_LATENCY */
      /* | LLL_DEBUG */;

  if ((p = lws_cmdline_option(argc, argv, "-d")))
    logs = atoi(p);

  lws_set_log_level(logs, NULL);

  memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
  info.port = port;
  info.mounts = &mount;
  info.protocols = protocols;
  info.vhost_name = "localhost";
  // info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

  info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
  info.ssl_cert_filepath = cert;
  info.ssl_private_key_filepath = key;

  if (lws_cmdline_option(argc, argv, "-h"))
    info.options |= LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK;

  if (lws_cmdline_option(argc, argv, "-v"))
    info.retry_and_idle_policy = &retry;

  context = lws_create_context(&info);
  if (!context)
  {
    lwsl_err("lws init failed\n");
    return false;
  }

  while (n >= 0 && globals_continue(NULL))
    n = lws_service(context, 0);

  lws_context_destroy(context);

  return true;
}
