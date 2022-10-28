include config.mk

FILES = orpheus drw util
SRC = $(addprefix ./src/, $(addsuffix .c, $(FILES)))
OBJ = $(addprefix ./build/, $(addsuffix .o, $(FILES)))

all: orpheus

build/%.o: src/%.c src/config.h
	${CC} -c ${CFLAGS} $< -o $@

orpheus: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm orpheus
	rm ./build/*.o


install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f orpheus ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/orpheus
	ln -s ${DESTDIR}${PREFIX}/bin/orpheus ${DESTDIR}${PREFIX}/bin/emoji-picker

	rm orpheus
	rm ./build/*.o

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/orpheus
	rm -f ${DESTDIR}${PREFIX}/bin/emoji-picker

.PHONY: all clean install uninstall
