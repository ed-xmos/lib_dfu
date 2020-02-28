#!/bin/sh
# observed 8us for the erase call followed by 80ms of actual erase time
# set thresholds at 100us and 100ms, respectively
echo timing_erase_sector_async
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe --upgrade 1 ../hello_world.xe || exit $?
xrun --io --args bin/timing_erase_sector_async.xe 10000 10000000
