/*
 * Crochet
 * An open source crypto currency router and agregated feed broker.
 * (C) washcloth et al.
 */

#include <httpws/wss.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pprint.h>
#include <stdio.h>

#include "pprint.h"

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	pprint_info(
	    "crochet (C) washcloth et al.", __FILE_NAME__, __func__, __LINE__);

	pprint_info("initializing OpenSSL", __FILE_NAME__, __func__, __LINE__);

	SSL_load_error_strings();
	SSL_library_init();

	struct wss_session google;
	if (wss_client("ws-feed.pro.coinbase.com", "/", "443", &google) != 0) {
		pprint_error("unable to make connection to coinbase",
		    __FILE_NAME__, __func__, __LINE__);
		return 1;
	}

	unsigned char *full_subscribe = (unsigned char
		*)"{\"type\":\"subscribe\",\"product_ids\":[\"BTC-USD\"],"
		  "\"channels\":[\"full\"]}";
	pprint_info("%s\n", __FILE_NAME__, __func__, __LINE__, full_subscribe);

	// for testing subscribe to the full web socket btc usd
	// for coinbase after we send this we can call wss_read to read the
	// stream of packets.
	wss_send_text(&google, full_subscribe, strlen((char *)full_subscribe));

  while(wss_read_text(&google) == WSS_ERR_NONE);

	return 0;
}
