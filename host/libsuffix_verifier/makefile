# Copyright (c) 2020, XMOS Ltd, All rights reserved
CFLAGS = -g -O2 -Wall -I ../api
LIBRARY = lib/libsuffix_verifier.a
MKDIR ?= mkdir
AR ?= ar

OBJS = suffix_verifier.o crc.o

all: dirs ${LIBRARY}

dirs:
	@${MKDIR} -p lib obj

${LIBRARY}: $(addprefix obj/,${OBJS})
	${AR} ru $@ $^

obj/%.o: src/%.c
	${CC} -c ${CFLAGS} $^ -o $@

clean:
	${RM} obj/*.o ${LIBRARY}
