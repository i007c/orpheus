include config.mk

SRC = drw.c orpheus.c util.c
OBJ = ${SRC:%.c=./build/%.o}

all: orpheus

build/%.o: %.c
	${CC} -c ${CFLAGS} $< -o $@

orpheus: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}


.PHONY: all 
