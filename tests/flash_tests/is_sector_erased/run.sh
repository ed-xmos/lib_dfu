#!/bin/sh
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe
xrun --io --args bin/is_sector_erased.xe N
xflash --no-compression --noinq --target=XCORE-200-EXPLORER --erase-all
xrun --io --args bin/is_sector_erased.xe E
