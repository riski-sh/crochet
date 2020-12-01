/*
 * ws protocol handler plugin for "lws-minimal"
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This version holds a single message at a time, which may be lost if a new
 * message comes.  See the minimal-ws-server-ring sample for the same thing
 * but using an lws_ring ringbuffer to hold up to 8 messages at a time.
 */

#include <pprint/pprint.h>
#if !defined(LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <web/comms.h>
#include <string.h>
#include <pthread.h>

/* one of these created for each message */

struct msg
{
  void *payload; /* is malloc'd */
  size_t len;
};

/* one of these is created for each client connecting to us */

struct per_session_data__minimal
{
  struct per_session_data__minimal *pss_list;
  struct lws *wsi;

  int last; /* the last message number we sent */
  struct msg amsg; /* the one pending message... */
  int current;     /* the current message number we are caching */
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data__minimal
{
  struct lws_context *context;
  struct lws_vhost *vhost;
  const struct lws_protocols *protocol;

  struct per_session_data__minimal *pss_list; /* linked-list of live pss*/
  int current;
};

/* destroys the message when everyone has had a copy of it */

static void
__minimal_destroy_message(void *_msg)
{
  struct msg *msg = _msg;

  free(msg->payload);
  msg->payload = NULL;
  msg->len = 0;
}

static int
callback_minimal(struct lws *wsi, enum lws_callback_reasons reason, void *user,
                 void *in, size_t len)
{
  struct per_session_data__minimal *pss =
      (struct per_session_data__minimal *)user;
  struct per_vhost_data__minimal *vhd =
      (struct per_vhost_data__minimal *)lws_protocol_vh_priv_get(
          lws_get_vhost(wsi), lws_get_protocol(wsi));
  int m;

  /*
   * push to clang to ignore the fact that we haven't cought every switch
   * statement
   */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"

  switch (reason)
  {
  case LWS_CALLBACK_PROTOCOL_INIT:
    vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi),
                                      sizeof(struct per_vhost_data__minimal));
    vhd->context = lws_get_context(wsi);
    vhd->protocol = lws_get_protocol(wsi);
    vhd->vhost = lws_get_vhost(wsi);
    break;

  case LWS_CALLBACK_ESTABLISHED:
    /* add ourselves to the list of live pss held in the vhd */
    lws_ll_fwd_insert(pss, pss_list, vhd->pss_list) pss->wsi = wsi;
    pss->last = vhd->current;
    break;

  case LWS_CALLBACK_CLOSED:
    /* remove our closing pss from the list of live pss */
    lws_ll_fwd_remove(struct per_session_data__minimal, pss_list, pss,
                      vhd->pss_list) break;

  case LWS_CALLBACK_SERVER_WRITEABLE:
    if (!pss->amsg.payload)
      break;

    if (pss->last == vhd->current)
      break;

    /* notice we allowed for LWS_PRE in the payload already */
    m = lws_write(wsi, ((unsigned char *)pss->amsg.payload) + LWS_PRE,
                  pss->amsg.len, LWS_WRITE_TEXT);
    if (m < (int)pss->amsg.len)
    {
      lwsl_err("ERROR %d writing to ws\n", m);
      exit(1);
    }

    __minimal_destroy_message(&pss->amsg);

    pss->last = vhd->current;
    break;

  case LWS_CALLBACK_RECEIVE:

    /* clear the last payload if any that was sent to the client */
    if (pss->amsg.payload)
      __minimal_destroy_message(&pss->amsg);

    /* modify payload to be c null terminiated string */
    ((char*)(in))[len] = '\x0';

    char *response = NULL;
    size_t msg_length = 0;

    comms_parse_message(in, &response, &msg_length);

    if (response) {
      pss->amsg.payload = malloc(LWS_PRE + msg_length);
      memcpy((char*)pss->amsg.payload + LWS_PRE, response, msg_length);
      pss->amsg.len = strlen(response);
      free(response);

      pss->current++;
      vhd->current++;
      lws_callback_on_writable(pss->wsi);
    } else {
      pss->amsg.payload = NULL;
      pss->amsg.len = 0;
    }

    break;
  default:
    break;
  }

#pragma clang diagnostic pop

  return 0;
}

#define LWS_PLUGIN_PROTOCOL_MINIMAL                                            \
  {                                                                            \
    "lws-minimal", callback_minimal, sizeof(struct per_session_data__minimal), \
        128, 0, NULL, 0                                                        \
  }
