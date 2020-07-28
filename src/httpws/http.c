#include <stdlib.h>

#include "http.h"

#define HTTP_WSS_UPGRADE_FMT            \
	"GET %s HTTP/1.1\r\n"           \
	"Host: %s\r\n"                  \
	"Upgrade: websocket\r\n"        \
	"Connection: Upgrade\r\n"       \
	"Sec-WebSocket-Key: %s\r\n"     \
	"Sec-WebSocket-Version: 13\r\n" \
	"\r\n"

struct _http_response {
	char *header_name;
	char *header_value;

	struct _http_response *next;
};

enum _http_parse_state {
	_http_parse_state_init = 0,
	_http_parse_state_header = 1,
	_http_parse_state_value = 2,
	_http_parse_state_end = 3
};

static void
_http_response_free(struct _http_response *res)
{
	while (res) {
		free(res->header_name);
		free(res->header_value);
		struct _http_response *cur = res;
		res = res->next;
		free(cur);
	}
}

/*
 * A private function that reads a line of the http headers response
 * @param ssl the ssl context to read from
 * @return a c string on the heap that is the callers responsibility to free.
 */
static char *
_http_ssl_getline(SSL *ssl)
{
	char *buf = NULL;
	size_t buf_len = 0;

	char cb = '\x0';
	while (SSL_read(ssl, &cb, 1) == 1) {
		if (cb == '\r') {
			SSL_read(ssl, &cb, 1);
			break;
		} else {
			buf_len += 1;
			buf = realloc(buf, sizeof(char) * buf_len);
			buf[buf_len - 1] = cb;
		}
	}

	if (buf_len == 0) {
		return NULL;
	}

	buf_len += 1;
	buf = realloc(buf, sizeof(char) * buf_len);
	buf[buf_len - 1] = '\x0';

	return buf;
}

static struct _http_response *
_http_parse_response(SSL *ssl)
{
	char *line = _http_ssl_getline(ssl);
	struct _http_response *response = malloc(sizeof(struct _http_response));

	response->header_name = line;
	response->header_value = NULL;
	response->next = NULL;

	struct _http_response *cur = response;

	while ((line = _http_ssl_getline(ssl)) && line != NULL) {
		cur->next = malloc(sizeof(struct _http_response));
		cur = cur->next;

		char *name = NULL;
		char *value = NULL;

		size_t line_len = 0;
		if (line) {
			line_len = strlen(line);
		} else {
			pprint_error("can not take strlen of NULL ptr",
			    __FILE_NAME__, __func__, __LINE__);
			abort();
		}

		size_t col_idx = 0;
		for (; col_idx < line_len; ++col_idx) {
			if (line[col_idx] == ':') {
				break;
			}
		}
		if (col_idx == line_len) {
			pprint_error("invalid http response %s", __FILE_NAME__,
			    __func__, __LINE__, line);
			abort();
		}

		name = malloc(sizeof(char) * (col_idx + 1));
		if (!name) {
			pprint_error("no more memory", __FILE_NAME__, __func__,
			    __LINE__);
			abort();
		}

		for (size_t i = 0; i < col_idx; ++i) {
			name[i] = line[i];
		}
		name[col_idx] = '\x0';

		value = malloc(sizeof(char) * line_len);
		if (!value) {
			pprint_error("no more memory", __FILE_NAME__, __func__,
			    __LINE__);
			abort();
		}

		for (size_t i = (col_idx + 2); i < line_len; ++i) {
			value[i - (col_idx + 2)] = line[i];
		}
		value[line_len - col_idx - 2] = '\x0';

		free(line);
		cur->header_name = name;
		cur->header_value = value;
		cur->next = NULL;
	}

	cur->next = NULL;

	return response;
}

int
http_wss_upgrade(SSL *ssl, char *endpoint, char *path)
{

	/*
	 * example http upgrade request
	 *
	 * GET /chat HTTP/1.1
	 * Host: server.example.com
	 * Upgrade: websocket
	 * Connection: Upgrade
	 * Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
	 * Sec-WebSocket-Protocol: chat, superchat
	 * Sec-WebSocket-Version: 13
	 */

	// Generate a 16 byte random number to be used as a communication feed.
	unsigned char raw_key[HTTP_WSS_KEY_LEN];

#ifdef _WIN32
#pragma error windows has no / dev / urandom.
#endif

	FILE *urandom = fopen("/dev/urandom", "rb");
	for (size_t i = 0; i < HTTP_WSS_KEY_LEN; ++i) {
		raw_key[i] = (unsigned char)fgetc(urandom);
	}
	fclose(urandom);

	pprint_info("generated crypto safe key "
		    "0x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x",
	    __FILE_NAME__, __func__, __LINE__, raw_key[0], raw_key[1],
	    raw_key[2], raw_key[3], raw_key[4], raw_key[5], raw_key[6],
	    raw_key[7], raw_key[8], raw_key[9], raw_key[10], raw_key[11],
	    raw_key[12], raw_key[13], raw_key[14], raw_key[15]);

	unsigned char *key_encoded = NULL;
	key_encoded = base64(raw_key, HTTP_WSS_KEY_LEN);

	pprint_info("generated Sec-WebSocket-Key: %s", __FILE_NAME__, __func__,
	    __LINE__, key_encoded);

	int req_size = snprintf(
	    NULL, 0, HTTP_WSS_UPGRADE_FMT, path, endpoint, key_encoded);

	char *request =
	    (char *)malloc(((unsigned long)req_size + 1) * sizeof(char));
	sprintf(request, HTTP_WSS_UPGRADE_FMT, path, endpoint, key_encoded);

	// don't send over the NULL terminator
	SSL_write(ssl, request, req_size);

	// get the response headers with values and find the
	// Sec-WebSocket-Accept header.
	struct _http_response *responses = _http_parse_response(ssl);
	struct _http_response *iter = responses;
	while (strcmp(iter->header_name, "Sec-WebSocket-Accept") != 0) {
		iter = iter->next;
	}

	if (iter) {
		pprint_info(
		    "confirmation key %s received web socket upgrade complete ",
		    __FILE_NAME__, __func__, __LINE__, iter->header_value);
	} else {
		pprint_error("server did not accept websocket upgrade",
		    __FILE_NAME__, __func__, __LINE__);
		abort();
	}

	free(key_encoded);
	_http_response_free(responses);
	return 0;
}
