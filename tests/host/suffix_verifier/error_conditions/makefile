# Copyright (c) 2020, XMOS Ltd, All rights reserved
DUT_PATH = ../../../../host/libsuffix_verifier
CFLAGS = -g -O2 -Wall -I ${DUT_PATH}
LDFLAGS = -L ${DUT_PATH}/lib -lsuffix_verifier
EXECUTABLE = bin/error_conditions
MKDIR ?= mkdir

TEST_OBJS = error_conditions.o

all: dirs lib ${EXECUTABLE}

dirs:
	@${MKDIR} -p bin obj

lib:
	make -C ${DUT_PATH}

${EXECUTABLE}: $(addprefix obj/,${TEST_OBJS})
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

obj/%.o: %.c
	${CC} -c ${CFLAGS} $^ -o $@

obj/%.o: ${DUT_PATH}/%.c
	${CC} -c ${CFLAGS} $^ -o $@

clean:
	make -C ${DUT_PATH} clean
	${RM} obj/*.o ${EXECUTABLE}
