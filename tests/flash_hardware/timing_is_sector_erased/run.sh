#!/bin/sh
echo timing_is_sector_erased

# empty flash
# observed 2.1ms so set threshold at 5ms
xflash --erase-all --target=XCORE-200-EXPLORER || exit $?
xrun --io --args bin/timing_is_sector_erased.xe 500000

# first few sectors programmed
# observed 113us so set threshold at 1ms
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe || exit $?
xrun --io --args bin/timing_is_sector_erased.xe 100000
