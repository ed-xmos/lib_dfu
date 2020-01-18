# Copyright (c) 2020, XMOS Ltd, All rights reserved
CFLAGS = -g -O2 -Wall -I ../../lib_dfu/api
EXECUTABLE = bin/suffix_generator
MKDIR ?= mkdir

OBJS = suffix_generator.o crc.o

all: dirs ${EXECUTABLE}

dirs:
	@${MKDIR} -p bin obj

${EXECUTABLE}: $(addprefix obj/,${OBJS})
	${CC} ${CFLAGS} $^ -o $@

obj/%.o: %.c
	${CC} -c ${CFLAGS} $^ -o $@

clean:
	${RM} obj/*.o ${EXECUTABLE}
