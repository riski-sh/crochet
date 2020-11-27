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

CFILES  != find src/ -name "*.c"

.c.o:
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $@

.SUFFIXES: .so
.c.so:
	${CC} -shared ${CFLAGS} ${IFLAGS} -fPIC src/api.c src/finmath/linear_equation.c $< -o $@

SERVER: src/web/web.o

EXCHANGES: src/exchanges/exchanges.o \
						src/exchanges/coinbase.o 	\
						src/exchanges/oanda.o

FFJSON: src/ffjson/ffjson.o

FINMATH: src/finmath/base_conversion.o \
					src/finmath/linear_equation.o

GLOBALS: src/globals/globals.o

HASHMAP: src/hashmap/hashmap.o

HTTPWS: src/httpws/base64.o 	\
	src/httpws/http11.o	\
	src/httpws/session.o \
	src/httpws/wss.o

ORDERBOOKS: src/orderbooks/book.o \
						 src/orderbooks/coinbase.o

PPRINT: src/pprint/pprint.o

SECURITY: src/security/analysis.o	\
					 src/security/chart.o			\
					 src/security/security.o

LIBS: libs/black_marubuzu.so \
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

libs: LIBS

all: SERVER EXCHANGES FFJSON GLOBALS HASHMAP HTTPWS ORDERBOOKS \
		 PPRINT SECURITY  libs   compile_commands
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) ${CWD}/src/**/*.o src/main.c src/api.c -o crochet.bin

.PHONY: compile_commands
compile_commands:
	@echo "[" > compile_commands.json
	@for f in ${CFILES}; do \
		echo "  {" >> compile_commands.json ; \
		echo "    \"directory\": \"${CWD}\"," >> compile_commands.json ; \
		echo "    \"command\":   \"${CC} -c ${CFLAGS} ${IFLAGS} $${f}\"," >> compile_commands.json; \
		echo "    \"file\":      \"$${f}\"" >> compile_commands.json ; \
		echo "  }," >> compile_commands.json ; \
	done
	@truncate -s-2 compile_commands.json
	@echo "" >> compile_commands.json
	@echo "]" >> compile_commands.json

clean:
	find libs/ -name "*.so" | xargs rm
	find src/ -name "*.o" | xargs rm
