# Global Namespace



## Records

hashmap

_map_list

candle

chart_object

chart_region

chart

security

json_array

json_value

generic_book

coinbase_value

coinbase_book_level

httpwss_session

_wss_packet

_http_response



## Functions

### globals_continue

*_Bool globals_continue(_Bool * val)*

*Defined at line 3 of /src/globals.c*

 A global infinite loop boolean

### btctosat_str

*uint64_t btctosat_str(char * num)*

*Defined at line 3 of /src/finmath/base_conversion.c*

 Converts a bitcoin value into its satoshi equivalent

### usdtocent_str

*uint64_t usdtocent_str(char * num)*

*Defined at line 13 of /src/finmath/base_conversion.c*

 Converts a USD dollar amount into its cents equivalent

### pprint_info

*void pprint_info(const char * str)*

*Defined at line 44 of /src/pprint.c*

 Prints out a log message under the level info

**str**

### pprint_warn

*void pprint_warn(const char * str)*

*Defined at line 61 of /src/pprint.c*

 Prints out a log message under the level warning

**str**

### pprint_error

*void pprint_error(const char * str)*

*Defined at line 77 of /src/pprint.c*

 Prints out a log message under the level error

**str**

### hashmap_new

*struct hashmap * hashmap_new(uint64_t num_bins)*

*Defined at line 30 of /src/hashmap/hashmap.c*

 Creates a new hashmap

**num_bins**

**return** a newly created hashmap

### hashmap_free

*void hashmap_free(struct hashmap * map)*

*Defined at line 87 of /src/hashmap/hashmap.c*

 Frees the hashmap struct created by hashmap_new

**map**

### hashmap_put

*void hashmap_put(char * key, void * value, struct hashmap * map)*

*Defined at line 52 of /src/hashmap/hashmap.c*

 Puts a value in the hashmap

**key**

**value**

**map**

### hashmap_get

*void * hashmap_get(char * key, struct hashmap * map)*

*Defined at line 68 of /src/hashmap/hashmap.c*

 Gets a value out of the hashmap.

**key**

**map**

**return** the value or null if the key doesn't exist

### sdbm

*uint64_t sdbm(char * str)*

*Defined at line 3 of /src/hashmap/hashmap.c*

### _map_list_add

*void _map_list_add(struct _map_list ** list, size_t ori_key, void * value)*

*Defined at line 19 of /src/hashmap/hashmap.c*

### chart_new

*struct chart * chart_new()*

*Defined at line 64 of /src/security/chart.c*

 Creates a new chart

### chart_update

*void chart_update(struct chart * cht, uint32_t bid, uint32_t ask, size_t timestamp)*

*Defined at line 109 of /src/security/chart.c*

 Updates the current chart



**cht**

**bid**

**ask**

### _chart_update_candle

*void _chart_update_candle(struct chart * cht, uint32_t bid, size_t idx)*

*Defined at line 26 of /src/security/chart.c*

 Updates a candle on the chart



**cht**

**bid**

**idx**

### _tstodow

*dow_t _tstodow(size_t timestamp)*

*Defined at line 101 of /src/security/chart.c*

 Converts a nanosecond timestamp to the day of week

### security_new

*struct security * security_new(char * name, int pip_location, int display_precision)*

*Defined at line 3 of /src/security/security.c*

 Creates a new security



**name**

**pip_location**

**display_precision**

### security_update

*_Bool security_update(struct security * sec, size_t timestamp, char * best_bid, char * best_ask)*

*Defined at line 15 of /src/security/security.c*

 Updates a security given a tick information

### exchange_init

*void exchange_init()*

*Defined at line 5 of /src/exchanges/exchanges.c*

### exchange_put

*void exchange_put(char * name, struct security * sec)*

*Defined at line 11 of /src/exchanges/exchanges.c*

### exchange_get

*struct security * exchange_get(char * name)*

*Defined at line 23 of /src/exchanges/exchanges.c*

### json_get_object

*__json_object json_get_object(__json_value val)*

*Defined at line 396 of /src/ffjson/ffjson.c*

### json_get_array

*__json_array json_get_array(__json_value val)*

*Defined at line 402 of /src/ffjson/ffjson.c*

### json_get_number

*__json_number json_get_number(__json_value val)*

*Defined at line 408 of /src/ffjson/ffjson.c*

### json_get_string

*__json_string json_get_string(__json_value val)*

*Defined at line 414 of /src/ffjson/ffjson.c*

### json_get_bool

*__json_bool json_get_bool(__json_value val)*

*Defined at line 420 of /src/ffjson/ffjson.c*

### json_parse

*__json_value json_parse(char * str)*

*Defined at line 329 of /src/ffjson/ffjson.c*

 Parses a json object

**str**

### json_parse_cached

*void json_parse_cached(char * str, size_t * idx, __json_value tree)*

*Defined at line 346 of /src/ffjson/ffjson.c*

 Takes an existing parse tree and given a json of the same structure will instead just fill in the new values and not create an entirely new json object. THIS IS HIGHLEY RECOMMENDED IF YOU ARE POLLING AND RECEIVING THE SAME JSON OBJECT BACK.

### json_free

*void json_free(__json_value root)*

*Defined at line 435 of /src/ffjson/ffjson.c*

 Frees a json object and all its children

**root**

### _parse_value

*__json_value _parse_value(char * str, size_t * idx)*

*Defined at line 254 of /src/ffjson/ffjson.c*

 fwd declaration of the few functions needed to be recursive in the processing

### _parse_whitespace

*void _parse_whitespace(char * str, size_t * idx)*

*Defined at line 11 of /src/ffjson/ffjson.c*

### _parse_value_seperator

*_Bool _parse_value_seperator(char * str, size_t * idx)*

*Defined at line 20 of /src/ffjson/ffjson.c*

### _parse_string

*__json_string _parse_string(char * str, size_t * idx)*

*Defined at line 53 of /src/ffjson/ffjson.c*

__always_inline static bool_valid_character(char *str, size_t *idx){  char c = str[*idx];

  if (c != '"') {    (*idx) += 1;    return true;  }

  char b = str[(*idx) - 1];  if (b == '\') {    (*idx) += 1;    return true;  }

  return false;}

### _parse_member

*void _parse_member(char * str, size_t * idx, char ** name, __json_value * _val, __json_object cached)*

*Defined at line 87 of /src/ffjson/ffjson.c*

### book_query

*struct generic_book * book_query(struct generic_book ** root, uint64_t price)*

*Defined at line 287 of /src/orderbooks/book.c*

 Inserts a key into the AVL tree and returns the new node that represents the tree. If the node already exists this will return the node with the represented price query.



**root**

**price**

**return** the node that represents this price level

### _parse_object

*__json_object _parse_object(char * str, size_t * idx, __json_object cached)*

*Defined at line 113 of /src/ffjson/ffjson.c*

### book_remove

*void book_remove(struct generic_book ** root, uint64_t price, book_free_data free_func)*

*Defined at line 447 of /src/orderbooks/book.c*

 Removes a price level from the book.



**root**

**price**

**free_func**

### _parse_array

*__json_array _parse_array(char * str, size_t * idx, __json_array cached)*

*Defined at line 177 of /src/ffjson/ffjson.c*

### book_free

*void book_free(struct generic_book * node, book_free_data free_func)*

*Defined at line 463 of /src/orderbooks/book.c*

 Frees an generic book and calls a book_free_data function to free the generic data inside the node

### _parse_number

*__json_number _parse_number(char * str, size_t * idx, __json_number cached)*

*Defined at line 232 of /src/ffjson/ffjson.c*

### book_print

*void book_print(struct generic_book * node)*

*Defined at line 6 of /src/orderbooks/book.c*

 Not very useful but prints the nodes and there children to be used in the graphviz software for viewing graphs



**node**

### test_book_query

*void test_book_query()*

*Defined at line 478 of /src/orderbooks/book.c*

### coinbase_book_received

*void coinbase_book_received(coinbase_book ** book, uint64_t price, struct coinbase_value * e)*

*Defined at line 72 of /src/orderbooks/coinbase.c*

 Primes the order for the order book



**book**

**price**

**e**

### coinbase_book_get

*void coinbase_book_get(coinbase_book * book, book_t book_type, int num, struct coinbase_book_level * data)*

*Defined at line 176 of /src/orderbooks/coinbase.c*

 Gets a limited number of book entries. This will return the best book entries from greatest to least or least to greatest depending on the book_type. This will populate an array of the values found.



**book**

**book_type**

**num**

**data**

### coinbase_book_open

*void coinbase_book_open(coinbase_book ** book, uint64_t price, uint64_t remaining, char * uuid)*

*Defined at line 101 of /src/orderbooks/coinbase.c*

### coinbase_book_remove

*void coinbase_book_remove(coinbase_book ** book, uint64_t price, char * uuid)*

*Defined at line 27 of /src/orderbooks/coinbase.c*

### coinbase_book_match

*void coinbase_book_match(coinbase_book ** book, uint64_t price, uint64_t size, char * maker_id)*

*Defined at line 3 of /src/orderbooks/coinbase.c*

### coinbase_book_value_free

*void coinbase_book_value_free(void * data)*

*Defined at line 194 of /src/orderbooks/coinbase.c*

 Frees the coinbase_value structure.

### _book_remove

*void _book_remove(struct generic_book ** root, struct generic_book * level, book_free_data free_func, _Bool no_free)*

*Defined at line 423 of /src/orderbooks/book.c*

### _max_depth

*int _max_depth(struct generic_book * node)*

*Defined at line 29 of /src/orderbooks/book.c*

### _ll_rotation

*void _ll_rotation(struct generic_book ** node)*

*Defined at line 49 of /src/orderbooks/book.c*

 Performs an ll rotation and returns the new root

### _rr_rotation

*void _rr_rotation(struct generic_book ** node)*

*Defined at line 82 of /src/orderbooks/book.c*

 Performs a right rotation and returns the new root

### _lr_rotation

*void _lr_rotation(struct generic_book ** node)*

*Defined at line 114 of /src/orderbooks/book.c*

 Performs an lr rotation

### _rl_rotation

*void _rl_rotation(struct generic_book ** node)*

*Defined at line 138 of /src/orderbooks/book.c*

 Performs an rl rotation

### _get_increasing

*void _get_increasing(coinbase_book * book, int * cur, int num, struct coinbase_book_level * d)*

*Defined at line 128 of /src/orderbooks/coinbase.c*

### _balance

*void _balance(struct generic_book ** node, _Bool from_delete)*

*Defined at line 159 of /src/orderbooks/book.c*

### _get_decreasing

*void _get_decreasing(coinbase_book * book, int * cur, int num, struct coinbase_book_level * d)*

*Defined at line 152 of /src/orderbooks/coinbase.c*

### _book_query

*struct generic_book * _book_query(struct generic_book ** _root, uint64_t price)*

*Defined at line 214 of /src/orderbooks/book.c*

### _get_successor

*struct generic_book * _get_successor(struct generic_book * b)*

*Defined at line 272 of /src/orderbooks/book.c*

### _remove_no_children

*void _remove_no_children(struct generic_book ** root, struct generic_book * level, book_free_data free_func, _Bool no_free)*

*Defined at line 303 of /src/orderbooks/book.c*

### _remove_one_child

*void _remove_one_child(struct generic_book ** root, struct generic_book * level, book_free_data free_func, _Bool no_free)*

*Defined at line 329 of /src/orderbooks/book.c*

### _remove_two_children

*void _remove_two_children(struct generic_book ** root, struct generic_book * level, book_free_data free_func)*

*Defined at line 387 of /src/orderbooks/book.c*

### _pprint_lock

*void _pprint_lock(FILE * fp)*

*Defined at line 9 of /src/pprint.c*

 Locks a file for writing

**fp**

### _pprint_unlock

*void _pprint_unlock(FILE * fp)*

*Defined at line 19 of /src/pprint.c*

 Unlocks a file for writing

**fp**

### _pprint_time

*void _pprint_time(char (*)[35] time)*

*Defined at line 29 of /src/pprint.c*

 Gets the time as a string in a standard format for logging.

**time**

### base64

*unsigned char * base64(const unsigned char * input, int length)*

*Defined at line 3 of /src/httpws/base64.c*

### decode64

*unsigned char * decode64(const unsigned char * input, int length)*

*Defined at line 16 of /src/httpws/base64.c*

### http_wss_upgrade

*int http_wss_upgrade(struct httpwss_session * session, char * path)*

*Defined at line 266 of /src/httpws/http.c*

 Upgrades an HTTP session to web socket. This is done by sending an HTTP upgrade request. This function will send the request and verify the response.



**session**

**path**

**return** Returns 0 on success and 1 if wss_upgrade was no successful

### http_get_request

*void http_get_request(struct httpwss_session * session, char * path, char ** res)*

*Defined at line 331 of /src/httpws/http.c*

 Performs an HTTP get request This function assumes an SSL/TLS connection and will fail if used on non secure tunnel.



**session**

**path**

**res**

### http_get_request_generate

*char * http_get_request_generate(struct httpwss_session * session, char * path)*

*Defined at line 364 of /src/httpws/http.c*

 Generates a the request header for performing an http get request. This can be useful if you have to keep on repeating the same request. The output of http_get_request_generate is ment to be passed along with http_get_request_cached which will not regenerate the get request and avoid expensive calls to snprintf.



**session**

**path**

### http_get_request_cached

*void http_get_request_cached(struct httpwss_session * session, char * request, int req_size, char ** response, char * record)*

*Defined at line 384 of /src/httpws/http.c*

 Uses a pre defined request to perform the http_get_request.



**session**

**request**

**req_size**

**response**

**record**



### httpwss_session_new

*struct httpwss_session * httpwss_session_new(char * endpoint, char * port)*

*Defined at line 401 of /src/httpws/http.c*

 Creates a new httpwss session to be used.



**endpoint**

**port**

### httpwss_session_free

*void httpwss_session_free(struct httpwss_session * session)*

*Defined at line 468 of /src/httpws/http.c*

 Cleans up the httpwss session that was created using the httpwss_session_new function

### exchanges_oanda_init

*void exchanges_oanda_init(char * key)*

*Defined at line 67 of /src/exchanges/oanda.c*

 Starts an oanda exchange feed



**key**

### _oanda_timetots

*size_t _oanda_timetots(char * str)*

*Defined at line 5 of /src/exchanges/oanda.c*

### _oanda_first_id

*char * _oanda_first_id(char * response)*

*Defined at line 13 of /src/exchanges/oanda.c*

### _oanda_gen_currency_list

*char * _oanda_gen_currency_list(char * response, int * num_instruments)*

*Defined at line 26 of /src/exchanges/oanda.c*

### wss_client

*enum WSS_ERR wss_client(char * endpoint, char * path, char * port, struct httpwss_session ** _session)*

*Defined at line 195 of /src/httpws/wss.c*

 Established a remote connection and upgrade the request from HTTP to web socket. This only establishes secure from HTTP to web socket. This only establishes secure TLS connections to the remote server.



**endpoint**

**port**

**_session**

### wss_send_text

*enum WSS_ERR wss_send_text(struct httpwss_session * session, unsigned char * text, size_t len)*

*Defined at line 154 of /src/httpws/wss.c*

### wss_read_text

*enum WSS_ERR wss_read_text(struct httpwss_session * session, char ** value)*

*Defined at line 60 of /src/httpws/wss.c*

### _wss_new_mask

*uint32_t _wss_new_mask()*

*Defined at line 23 of /src/httpws/wss.c*

 Generates a new mask to use for sending data

**return** a value to mask data with

### _wss_send_header

*void _wss_send_header(struct httpwss_session * session, struct _wss_packet * p)*

*Defined at line 38 of /src/httpws/wss.c*

 Write a web socket frame header to the session given a header After this is called masked data may be sent to the server.

### _http_parse_response

*struct _http_response * _http_parse_response(char ** res)*

*Defined at line 229 of /src/httpws/http.c*

### _http_response_free

*void _http_response_free(struct _http_response * res)*

*Defined at line 218 of /src/httpws/http.c*

### _http_ssl_read_all

*void _http_ssl_read_all(SSL * ssl, char ** _r, uint32_t * _n, char * _record)*

*Defined at line 54 of /src/httpws/http.c*

 Read everything into one buffer

### exchanges_coinbase_init

*void exchanges_coinbase_init()*

*Defined at line 351 of /src/exchanges/coinbase.c*

 Starts the coinbase feeds

### _load_config

*__json_value _load_config(char * file, char ** raw)*

*Defined at line 37 of /src/main.c*

### sig_handler

*void sig_handler(int sig)*

*Defined at line 63 of /src/main.c*

### main

*int main(int argc, char ** argv)*

*Defined at line 78 of /src/main.c*

### _subscribe

*void _subscribe(char * id, struct httpwss_session * coinbase_local)*

*Defined at line 3 of /src/exchanges/coinbase.c*

### _build_book

*coinbase_book * _build_book(__json_array side)*

*Defined at line 23 of /src/exchanges/coinbase.c*

### _parse_open

*void _parse_open(__json_object msg, coinbase_book ** bid, coinbase_book ** ask)*

*Defined at line 68 of /src/exchanges/coinbase.c*

### _parse_done

*void _parse_done(__json_object msg, coinbase_book ** bid, coinbase_book ** ask)*

*Defined at line 95 of /src/exchanges/coinbase.c*

### _parse_received

*void _parse_received(__json_object msg, coinbase_book ** bid, coinbase_book ** ask)*

*Defined at line 127 of /src/exchanges/coinbase.c*

### _parse_match

*void _parse_match(__json_object msg, coinbase_book ** bid, coinbase_book ** ask)*

*Defined at line 175 of /src/exchanges/coinbase.c*

### _coinbase_start

*void _coinbase_start(void * id)*

*Defined at line 206 of /src/exchanges/coinbase.c*

 Pulls the full order book and starts processing the order book values.

 The order of operations is as so.

 1. Start a web socket connection and let it start streaming data to us. 2. Pull the order book after starting the web socket connection. 3. Read the buffered messages and maintain the full book



**id**



## Enums

| enum  |

--

| CHART_REGION |
| CHART_LINE |


*Defined at line 39 of /src/security/chart.h*

 Every type of chart object that can be drawn.

| enum  |

--

| THURSDAY |
| FRIDAY |
| SATURDAY |
| SUNDAY |
| MONDAY |
| TUESDAY |
| WEDNESDAY |
| NUM_DOW_T |


*Defined at line 6 of /src/security/chart.c*

 Represents a day of week from the start of the epoch

| enum JSON_TYPE |

--

| JSON_TYPE_OBJECT |
| JSON_TYPE_ARRAY |
| JSON_TYPE_NUMBER |
| JSON_TYPE_STRING |
| JSON_TYPE_TRUE |
| JSON_TYPE_FALSE |
| JSON_TYPE_NULL |
| JSON_TYPE_NUM |


*Defined at line 21 of /src/ffjson/ffjson.h*

| enum  |

--

| RANGE_INCREASING |
| RANGE_DECREASING |


*Defined at line 10 of /src/orderbooks/book.h*

| enum  |

--

| BOOK_BID |
| BOOK_ASK |


*Defined at line 12 of /src/orderbooks/book.h*

| enum WSS_ERR |

--

| WSS_ERR_NONE |
| WSS_ERR_GET_ADDR_INFO |
| WSS_ERR_SOCKET_CREATION |
| WSS_ERR_CONNECT_FAILURE |
| WSS_ERR_NUM |


*Defined at line 26 of /src/httpws/wss.h*

| enum _http_parse_state |

--

| _http_parse_state_init |
| _http_parse_state_header |
| _http_parse_state_value |
| _http_parse_state_end |


*Defined at line 39 of /src/httpws/http.c*

| enum  |

--

| CONTENT_LENGTH |
| CHUNCKED |
| UNKNOWN |


*Defined at line 46 of /src/httpws/http.c*



