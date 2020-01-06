# Copyright (c) 2020, XMOS Ltd, All rights reserved
#
# typical build command:
#
# xcc -Wno-bidirectional-buffered-port \
#   -Xmapper --dontenablesodlinks -Xmapper --nochaninit -Xmapper --noinitialtidy \
#   -Xmapper --image-base -Xmapper 0x40080 -Xmapper --image-size -Xmapper 0x3ff80 \
#   -Xmapper --wno110 -Xmapper --wno226 -Xmapper --wnoXN \
#   -lstage2loader -lswitchsetup -lsqiaccess -lquadspi -llocks -lxcc \
#   -std=c99 -O2 target.xn stage2loader.xc loader.c
#

HEADERS = data_partition_structures.h
CP ?= cp

headers:
	@${CP} $(addprefix ../lib_dfu/api/,${HEADERS}) .
