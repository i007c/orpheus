include config.mk

SRC = drw.c orpheus.c util.c
OBJ = ${SRC:%.c=./build/%.o}

all: orpheus

build/%.o: %.c config.h
	${CC} -c ${CFLAGS} $< -o $@

orpheus: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm orpheus
	rm ./build/*.o

.PHONY: all clean
