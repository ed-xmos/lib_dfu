# Copyright (c) 2019, XMOS Ltd, All rights reserved
CFLAGS = -g -O2 -Wall -I ../../lib_dfu/api
EXECUTABLE = data_partition_generator

OBJS = data_partition_generator.o \
  options.o input_files.o output_file.o descriptions.o images.o \
  checksum.o crc.o

${EXECUTABLE}: ${OBJS}
	${CC} ${CFLAGS} $^ -o $@

%.o: %.c
	${CC} -c ${CFLAGS} $*.c -o $*.o

clean:
	${RM} *.o ${EXECUTABLE}
