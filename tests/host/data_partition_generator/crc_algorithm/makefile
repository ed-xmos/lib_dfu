# Copyright (c) 2019, XMOS Ltd, All rights reserved
DUT_PATH = ../../../../host/data_partition_generator
CFLAGS = -g -O2 -Wall -I ${DUT_PATH}
EXECUTABLE = crc_algorithm

DUT_OBJS = crc.o
TEST_OBJS = crc_algorithm.o

${EXECUTABLE}: ${TEST_OBJS} $(addprefix ${DUT_PATH}/,${DUT_OBJS})
	${CC} ${CFLAGS} $^ -o $@

%.o: %.c
	${CC} -c ${CFLAGS} $*.c -o $*.o

clean:
	${RM} *.o ${EXECUTABLE}
