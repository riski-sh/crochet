CC?=gcc
CFLAGS?=-Os -g -m64 $(shell pkgconf --cflags openssl) -lpthread
LFLAGS?=$(shell pkgconf --libs openssl)
WFLAGS?=-Wall -Wextra -Wpedantic -Werror
FFLAGS?=-fasynchronous-unwind-tables -fexceptions
IFLAGS?=-I$(shell pwd)/src
OBJDIR=$(shell pwd)/obj

export CC
export CFLAGS
export LFLAGS
export WFLAGS
export FFLAGS
export IFLAGS
export OBJDIR

all: objdir src/
	@$(MAKE) -C src/

objdir:
	mkdir -p obj

clean: objdir
	rm -rf $(OBJDIR)
