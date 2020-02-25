# Copyright (c) 2020, XMOS Ltd, All rights reserved

SUFFIX_GEN_DIR="../../host/suffix_generator"
all:
	find . -mindepth 3 -name makefile | while read f ; do \
	  echo `dirname +$$f` ; make -C `dirname $$f` all ; done

	cd $(SUFFIX_GEN_DIR); cmake  .; make

clean:
	find . -mindepth 3 -name makefile | while read f ; do \
	  echo `dirname +$$f` ; make -C `dirname $$f` clean ; done
	if [ -a $(SUFFIX_GEN_DIR)/CMakeCache.txt ]; then rm $(SUFFIX_GEN_DIR)/CMakeCache.txt; fi;
	if [ -a $(SUFFIX_GEN_DIR)/makefile ]; then rm $(SUFFIX_GEN_DIR)/makefile; fi;
