# Copyright (c) 2020, XMOS Ltd, All rights reserved
DUT_PATH = ../../../../host/suffix_generator
EXECUTABLE = suffix_generator
MKDIR ?= mkdir
CP ?= cp

all: dirs build

dirs:
	@${MKDIR} -p bin

build:
	make -C ${DUT_PATH} clean all
	${CP} ${DUT_PATH}/bin/${EXECUTABLE} bin

clean:
	${RM} ${EXECUTABLE}
	make -C ${DUT_PATH} clean
