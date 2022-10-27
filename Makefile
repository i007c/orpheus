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

.PHONY: all clean
