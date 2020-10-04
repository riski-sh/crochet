CC?=gcc
CFLAGS?=-Os -march=native $(shell pkgconf --cflags openssl)
LFLAGS?=$(shell pkgconf --libs openssl)
WFLAGS?=-Wall -Wextra -Wpedantic -Werror -Wl,-z,defs -Wl,-z,now -Wl,-z,relro
FFLAGS?=-fasynchronous-unwind-tables -fexceptions -fpie -Wl,-pie
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
