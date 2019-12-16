#!/bin/sh
echo sector_erase
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe --upgrade 1 ../hello_world.xe
xrun --io bin/sector_erase.xe
