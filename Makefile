CC  = clang

CWD != pwd

OPENSSL_CFLAGS != pkgconf --cflags openssl | xargs
OPENSSL_LIBS != pkgconf --libs openssl | xargs

CFLAGS  += -isystem /usr/local/include
CFLAGS  += -isystem ${CWD}/depends/libwebsockets/build/include/
CFLAGS  += -Weverything -Werror -Wno-padded
CFLAGS  += -O2 -g3
CFLAGS  != echo ${CFLAGS} | sort

IFLAGS  += -I${CWD}/src
IFLAGS  += -I${CWD}/depends/libwebsockets/build/include/
IFLAGS  != echo ${IFLAGS} | sort

LFLAGS  += ${OPENSSL_LIBS}
LFLAGS  += -L/usr/local/lib/ -lwebsockets -lpthread -lm -ldl
LFLAGS  != echo ${LFLAGS} | sort

OBJDIR  := ${CWD}/obj

%.o : %.c %.h
	@mkdir -p $(OBJDIR)/$(shell dirname $*)
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $(OBJDIR)/$@

.server: src/web/web.o

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
all : .exchanges .ffjson .finmath .globals .hashmap .httpws .server \
			.orderbooks .pprint .security .libs
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) ${OBJDIR}/src/**/*.o src/main.c src/api.c -o crochet.bin

libs : .libs

.PHONY: clean
clean :
	rm -rf obj/src/
