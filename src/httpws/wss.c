#include <netinet/in.h>
#include <byteswap.h>

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
	static FILE *fp = NULL;

	if (!fp) {
		fp = fopen("/dev/urandom", "rb");
	}

	uint32_t mask;
	fread(&mask, sizeof(uint32_t), 1, fp);
	return mask;
}

/*
 * Write a web socket frame header to the session given a header
 * After this is called masked data may be sent to the server.
 */
static void
_wss_send_header(struct wss_session *session, struct _wss_packet *p)
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
wss_read_text(struct wss_session *session)
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
		pprint_error("i don't know how to handle binary frames yet",
		    __FILE_NAME__, __func__, __LINE__);
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
		pprint_error("// TODO handle connection close", __FILE_NAME__,
		    __func__, __LINE__);
		abort();
	case 0x9:
		pprint_error(
		    "// TODO handle ping", __FILE_NAME__, __func__, __LINE__);
		abort();
	case 0xA:
		pprint_error(
		    "// TODO handle pong", __FILE_NAME__, __func__, __LINE__);
		abort();
	case 0xB:
	case 0xC:
	case 0xD:
	case 0xE:
	case 0xF:
		pprint_error(
		    "the standard has changed, i should probably update",
		    __FILE_NAME__, __func__, __LINE__);
    abort();
	default:
		pprint_error("0x%x is not a valid frame type", __FILE_NAME__,
		    __func__, __LINE__, h.opcode);
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

  char *to_read = malloc(sizeof(char) * (h2+1));
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

  printf("%s\n", to_read);
  free(to_read);
	return WSS_ERR_NONE;
}

enum WSS_ERR
wss_send_text(struct wss_session *session, unsigned char *text, uint64_t len)
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
		pprint_error("this packet can't fit in a single frame",
		    __FILE_NAME__, __func__, __LINE__);
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
wss_client(char *endpoint, char *path, char *port, struct wss_session *session)
{
	pprint_info("starting connection to %s:%s", __FILE_NAME__, __func__,
	    __LINE__, endpoint, port);

	// convert endpoint to an ip address
	struct addrinfo *res = NULL;

	if (getaddrinfo(endpoint, port, NULL, &res) != 0) {
		pprint_error("unable to resolve %s", __FILE_NAME__, __func__,
		    __LINE__, endpoint);
		return WSS_ERR_GET_ADDR_INFO;
	}

	// create the socket
	session->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (session->fd == -1) {
		pprint_error(
		    "unable to create socket probably because one is already open",
		    __FILE_NAME__, __func__, __LINE__);
		return WSS_ERR_SOCKET_CREATION;
	}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
#pragma clang diagnostic pop

	char ipAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);

	pprint_info("resolved %s to %s", __FILE_NAME__, __func__, __LINE__,
	    endpoint, ipAddress);

	// connect the socket to the remote host
	if (connect(session->fd, res->ai_addr, res->ai_addrlen) == -1) {
		return WSS_ERR_CONNECT_FAILURE;
	}

	// prime SSL for establishing TLS connection
	const SSL_METHOD *method = TLS_client_method();
	SSL_CTX *ctx = SSL_CTX_new(method);

	if (ctx == NULL) {
		ERR_print_errors_fp(stdout);
		abort();
	}
	session->ssl = SSL_new(ctx);
	SSL_set_fd(session->ssl, session->fd);
	SSL_set_tlsext_host_name(session->ssl, endpoint);

	// perform the TLS handshake
	if (SSL_connect(session->ssl) == -1) {
		ERR_print_errors_fp(stdout);
		abort();
	}

	pprint_info("tls handshake accepted by %s", __FILE_NAME__, __func__,
	    __LINE__, endpoint);

	// upgrade this connection to websocket
	pprint_info("upgrading HTTP session to web socket", __FILE_NAME__,
	    __func__, __LINE__, endpoint);

	http_wss_upgrade(session->ssl, endpoint, path);

	freeaddrinfo(res);

	return WSS_ERR_NONE;
}
