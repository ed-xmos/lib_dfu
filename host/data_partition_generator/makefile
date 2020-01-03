# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
CFLAGS = -g -O2 -Wall -I ../../lib_dfu/api
EXECUTABLE = bin/data_partition_generator
MKDIR ?= mkdir

OBJS = data_partition_generator.o \
  options.o input_files.o output_file.o descriptions.o images.o \
  checksum.o crc.o

all: dirs ${EXECUTABLE}

dirs:
	@${MKDIR} -p bin obj

${EXECUTABLE}: $(addprefix obj/,${OBJS})
	${CC} ${CFLAGS} $^ -o $@

obj/%.o: src/%.c
	${CC} -c ${CFLAGS} $^ -o $@

clean:
	${RM} obj/*.o ${EXECUTABLE}
