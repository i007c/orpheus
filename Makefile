# orpheus version
VERSION = 0.2.0

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# flags
CFLAGS   = -std=gnu11 -pedantic -Os
CFLAGS  += -Wall -Werror -Wextra
CFLAGS  += -Wno-missing-braces
CFLAGS  += -I/usr/X11R6/include -I/usr/include/freetype2 -Isrc/include
CFLAGS  += -DVERSION=\"${VERSION}\"
LDFLAGS  = -L/usr/X11R6/lib -lXft -lfontconfig -lX11

FILES = orpheus drw util
SRC = $(addprefix ./src/, $(addsuffix .c, $(FILES)))
OBJ = $(addprefix ./build/, $(addsuffix .o, $(FILES)))

all: clear orpheus

src/include/config.h:
	cp src/include/config.def.h $@

src/include/emojis.h:
	cp src/include/emojis.def.h $@

build/%.o: src/%.c src/include/config.h src/include/emojis.h
	${CC} -c ${CFLAGS} $< -o $@

orpheus: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}


clear:
	printf "\E[H\E[3J"
	clear

run: clear orpheus
	./orpheus

clean:
	rm orpheus
	rm ./build/*.o


install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f orpheus ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/orpheus
	ln -sf ${DESTDIR}${PREFIX}/bin/orpheus ${DESTDIR}${PREFIX}/bin/emoji-picker

	rm orpheus
	rm ./build/*.o

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/orpheus
	rm -f ${DESTDIR}${PREFIX}/bin/emoji-picker

.PHONY: all clean install uninstall run clear
.SILENT: run clean clear
