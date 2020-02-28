#!/bin/sh
# observed 336, 1,007 and 133 usec so set thresholds at 0.5, 2 and 0.5 msec, respectively
echo timing_page_write_and_verify
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe || exit $?
xrun --io --args bin/timing_page_write_and_verify.xe 50000 200000 50000
