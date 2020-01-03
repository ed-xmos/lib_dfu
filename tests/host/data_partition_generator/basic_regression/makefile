# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
DUT_PATH = ../../../../host/data_partition_generator
EXECUTABLE = data_partition_generator
CP ?= cp

all:
	make -C ${DUT_PATH}
	${CP} ${DUT_PATH}/bin/${EXECUTABLE} .

clean:
	${RM} ${EXECUTABLE}
