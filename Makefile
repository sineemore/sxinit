.POSIX:
.SUFFIXES:
NAME    = sxinit
CC      = gcc
CFLAGS  = -Os
LDFLAGS =
SRC     = $(NAME).c
OBJ     = ${SRC:.c=.o}


all: $(NAME)

sxinit: sxinit.c
	$(CC) $(CFLAGS) -o sxinit sxinit.c

install: all
	mkdir -p $(PREFIX)/bin
	cp -f $(NAME) $(PREFIX)/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(NAME)

.PHONY: clean
clean:
	rm -f -- $(NAME) $(OBJ)

sx: sxinit.c minilib.conf
	$(MINI_GCC) --config minilib.conf -o sxinit sxinit.c
