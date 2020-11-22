CC?=clang

CFLAGS+=-Weverything -Werror -pedantic -isystem /usr/local/include -O2 -g
CFLAGS+=-Wno-padded

IFLAGS=-I$(shell pwd)/src
IFLAGS+=$(shell pkgconf --cflags openssl | xargs)

IFLAGS:=$(sort $(IFLAGS))

LFLAGS=$(shell pkgconf --libs openssl | xargs)
LFLAGS:=$(sort $(LFLAGS)) -lpthread -lm -ldl

OBJDIR:=$(shell pwd)/obj

STRUCTURE=$(shell find src/ -type d)

%.o : %.c %.h
	@mkdir -p $(OBJDIR)/$(shell dirname $*)
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $(OBJDIR)/$@

.exchanges: src/exchanges/exchanges.o \
						src/exchanges/coinbase.o 	\
						src/exchanges/oanda.o

.ffjson: src/ffjson/ffjson.o

.finmath: src/finmath/base_conversion.o \
					src/finmath/linear_equation.o

.globals: src/globals/globals.o

.hashmap: src/hashmap/hashmap.o

.httpws: src/httpws/base64.o 	\
				 src/httpws/http11.o	\
				 src/httpws/session.o \
				 src/httpws/wss.o

.orderbooks: src/orderbooks/book.o \
						 src/orderbooks/coinbase.o

.pprint: src/pprint/pprint.o

.security: src/security/analysis.o	\
					 src/security/chart.o			\
					 src/security/security.o

%.so : %.c
	@mkdir -p $(OBJDIR)/libs
	$(CC) -shared $(CFLAGS) $(IFLAGS) -fPIC src/api.c src/finmath/linear_equation.c $< -o $(OBJDIR)/$@

.libs : libs/black_marubuzu.so \
				libs/dragonfly_doji.so \
				libs/four_price_doji.so \
				libs/gravestone_doji.so \
				libs/hanging_man.so \
				libs/long_legged_dragonfly_doji.so \
				libs/shooting_star.so \
				libs/spinning_top.so \
				libs/white_marubuzu.so \
				libs/support_trend.so \
				libs/resistance_trend.so

.PHONY: all
all : .exchanges .ffjson .finmath .globals .hashmap .httpws \
			.orderbooks .pprint .security .libs
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $(shell find obj/ -type f -name "*.o") src/main.c src/api.c -o crochet.bin

libs : .libs

.PHONY: clean
clean :
	rm -rf obj/src/
