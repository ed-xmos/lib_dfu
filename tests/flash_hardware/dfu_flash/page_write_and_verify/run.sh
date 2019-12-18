#!/bin/sh
echo page_write_and_verify
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe || exit $?
xrun --io bin/page_write_and_verify.xe
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe --upgrade 1 ../hello_world.xe || exit $?
xrun --io bin/page_write_and_verify.xe
