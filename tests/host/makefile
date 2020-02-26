# Copyright (c) 2020, XMOS Ltd, All rights reserved

all:
	find . -mindepth 3 -name makefile | while read f ; do \
	  echo `dirname +$$f` ; make -C `dirname $$f` all ; done

clean:
	find . -mindepth 3 -name makefile | while read f ; do \
	  echo `dirname +$$f` ; make -C `dirname $$f` clean ; done
