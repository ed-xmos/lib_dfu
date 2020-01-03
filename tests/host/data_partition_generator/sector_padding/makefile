# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
DUT_PATH = ../../../../host/data_partition_generator
DFU_PATH = ../../../../lib_dfu
CFLAGS = -g -O2 -Wall -I ${DUT_PATH}/src -I ${DFU_PATH}/api
EXECUTABLE = bin/sector_padding
MKDIR ?= mkdir

DUT_OBJS = images.o checksum.o crc.o
TEST_OBJS = sector_padding.o

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
