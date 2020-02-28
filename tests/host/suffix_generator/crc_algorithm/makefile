# Copyright (c) 2020, XMOS Ltd, All rights reserved
DUT_PATH = ../../../../host/suffix_generator/src
CFLAGS = -g -O2 -Wall -I ${DUT_PATH}
EXECUTABLE = bin/crc_algorithm
MKDIR ?= mkdir

DUT_OBJS = crc.o
TEST_OBJS = crc_algorithm.o

all: dirs ${EXECUTABLE}

dirs:
	@${MKDIR} -p bin obj

${EXECUTABLE}: $(addprefix obj/,${TEST_OBJS} ${DUT_OBJS})
	${CC} ${CFLAGS} $^ -o $@

obj/%.o: %.c
	${CC} -c ${CFLAGS} $^ -o $@

obj/%.o: ${DUT_PATH}/%.c
	${CC} -c ${CFLAGS} $^ -o $@

clean:
	${RM} obj/*.o ${EXECUTABLE}
