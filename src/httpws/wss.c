#include "wss.h"

struct _wss_packet {
  uint8_t fin;
  uint8_t rsv1;
  uint8_t rsv2;
  uint8_t rsv3;
  uint8_t opcode;
  uint8_t mask;
  uint8_t len;
  char _p1[1]; // pad here
  uint16_t len_ext;
  char _p2[6]; // pad here
  uint64_t len_ext_con;
  uint32_t mask_key;
  char _p3[4]; // pad here
};

/*
 * Generates a new mask to use for sending data
 * @return a value to mask data with
 */
static uint32_t
_wss_new_mask()
{

  FILE *fp = fopen("/dev/urandom", "rb");
  uint32_t mask;
  fread(&mask, sizeof(uint32_t), 1, fp);
  fclose(fp);
  return mask;
}

/*
 * Write a web socket frame header to the session given a header
 * After this is called masked data may be sent to the server.
 */
static void
_wss_send_header(struct httpwss_session *session, struct _wss_packet *p)
{
  // fin, rsv1, 2, 3, opcode
  unsigned char h1 = 0;
  h1 = (unsigned char)((p->fin << 7) | (p->rsv1 << 6) | (p->rsv2 << 5) |
      (p->rsv3 << 4) | (p->opcode));
  unsigned char h2 = 0;
  h2 = (unsigned char)((p->mask << 7) | (p->len));

  SSL_write(session->ssl, &h1, 1);
  SSL_write(session->ssl, &h2, 1);

  if (p->len == 126) {
    SSL_write(session->ssl, &(p->len_ext), sizeof(uint16_t));
  } else if (p->len == 127) {
    SSL_write(session->ssl, &(p->len_ext_con), sizeof(uint64_t));
  }

  SSL_write(session->ssl, &(p->mask_key), sizeof(uint32_t));
}

enum WSS_ERR
wss_read_text(struct httpwss_session *session, char **value)
{
  struct _wss_packet h;
  bzero(&h, sizeof(struct _wss_packet));

  unsigned char h1 = 0;
  if (SSL_read(session->ssl, &h1, 1) != 1) {
    return WSS_ERR_CONNECT_FAILURE;
  }
  h.fin = (h1 >> 7);
  h.rsv1 = (uint8_t)(h1 << 1) >> 7;
  h.rsv2 = (uint8_t)(h1 << 2) >> 7;
  h.rsv3 = (uint8_t)(h1 << 3) >> 7;
  h.opcode = (uint8_t)(h1 << 4) >> 4;

  switch (h.opcode) {
  case 0x0:
    pprint_error("i don't know how to handle continuation frames",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  case 0x1:
    break;
  case 0x2:
    pprint_error("i don't know how to handle binary frames yet", __FILE_NAME__,
        __func__, __LINE__);
    abort();
  case 0x3:
  case 0x4:
  case 0x5:
  case 0x6:
  case 0x7:
    pprint_error("i don't know how to handle your specific codes",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  case 0x8:
    pprint_error(
        "// TODO handle connection close", __FILE_NAME__, __func__, __LINE__);
    abort();
  case 0x9:
    pprint_error("// TODO handle ping", __FILE_NAME__, __func__, __LINE__);
    abort();
  case 0xA:
    pprint_error("// TODO handle pong", __FILE_NAME__, __func__, __LINE__);
    abort();
  case 0xB:
  case 0xC:
  case 0xD:
  case 0xE:
  case 0xF:
    pprint_error("the standard has changed, i should probably update",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  default:
    pprint_error("0x%x is not a valid frame type", __FILE_NAME__, __func__,
        __LINE__, h.opcode);
    abort();
  }

  // read the payload length next
  uint64_t h2 = 0;
  if (SSL_read(session->ssl, &h2, 1) != 1) {
    return WSS_ERR_CONNECT_FAILURE;
  }

  if (h2 == 126) {
    uint16_t to_read = 0;
    SSL_read(session->ssl, &to_read, sizeof(uint16_t));
    h2 = __bswap_16(to_read);
  } else if (h2 == 127) {
    uint64_t to_read = 0;
    SSL_read(session->ssl, &to_read, sizeof(uint64_t));
    h2 = __bswap_64(to_read);
  }

  char *to_read = malloc(sizeof(char) * (h2 + 1));
  for (uint64_t i = 0; i < h2; ++i) {
    char b;
    if (SSL_read(session->ssl, &b, 1) != 1) {
      pprint_error("add error checked to ssl read when it should be working",
          __FILE_NAME__, __func__, __LINE__);
      abort();
    }
    to_read[i] = b;
  }
  to_read[h2] = '\x0';

  *value = to_read;
  return WSS_ERR_NONE;
}

enum WSS_ERR
wss_send_text(
    struct httpwss_session *session, unsigned char *text, uint64_t len)
{
  struct _wss_packet to_send;
  bzero(&to_send, sizeof(struct _wss_packet));
  to_send.fin = 1;
  to_send.rsv1 = 0;
  to_send.rsv2 = 0;
  to_send.rsv3 = 0;
  to_send.opcode = 0x1;
  to_send.mask = 1;
  to_send.mask_key = _wss_new_mask();

  // create the initial header
  if (len <= 125) {
    to_send.len = (uint8_t)len;
  } else if (len > 125 && len <= USHRT_MAX) {
    to_send.len = 126;
    to_send.len_ext = (uint16_t)len;
  } else if (len > USHRT_MAX && len < ULONG_MAX) {
    to_send.len = 127;
    to_send.len_ext_con = len;
  } else {
    pprint_error("this packet can't fit in a single frame", __FILE_NAME__,
        __func__, __LINE__);
  }

  // send the header
  _wss_send_header(session, &to_send);

  // send the masked data
  for (uint64_t i = 0; i < len; ++i) {
    unsigned char mask = i % 4;
    mask = text[i] ^ ((unsigned char *)(&(to_send.mask_key)))[mask];
    SSL_write(session->ssl, &mask, 1);
  }

  return WSS_ERR_NONE;
}

enum WSS_ERR
wss_client(
    char *endpoint, char *path, char *port, struct httpwss_session **_session)
{

  struct httpwss_session *session = httpwss_session_new(endpoint, port);

  http_wss_upgrade(session, path);

  *_session = session;

  return WSS_ERR_NONE;
}
