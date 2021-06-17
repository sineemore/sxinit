.POSIX:
.SUFFIXES:
NAME    = sxinit
CC      = gcc
CFLAGS  = -Os
LDFLAGS =
SRC     = $(NAME).c
OBJ     = ${SRC:.c=.o}
MINI_GCC= mini-gcc

all: $(NAME)

sxinit: sxinit.c
	$(CC) $(CFLAGS) -o sxinit sxinit.c

install: all
	mkdir -p $(PREFIX)/bin
	cp -f $(NAME) $(PREFIX)/bin
	sudo chown root:root ${DESTDIR}${PREFIX}/bin/$(NAME)
	sudo chmod u+s ${DESTDIR}${PREFIX}/bin/$(NAME)

.PHONY: clean
clean:
	rm -f -- $(NAME) $(OBJ)

with-minilib:
	make -f minilib.conf


