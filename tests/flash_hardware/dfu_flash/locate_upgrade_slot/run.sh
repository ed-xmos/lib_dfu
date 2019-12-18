#!/bin/sh
echo locate_upgrade_slot
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe || exit $?
xrun --io --args bin/locate_upgrade_slot.xe 28672
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe --upgrade 1 ../hello_world.xe || exit $?
xrun --io --args bin/locate_upgrade_slot.xe 28672
