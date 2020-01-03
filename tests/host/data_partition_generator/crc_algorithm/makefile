# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
DUT_PATH = ../../../../host/data_partition_generator
CFLAGS = -g -O2 -Wall -I ${DUT_PATH}/src
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

obj/%.o: ${DUT_PATH}/src/%.c
	${CC} -c ${CFLAGS} $^ -o $@

clean:
	${RM} obj/*.o ${EXECUTABLE}
