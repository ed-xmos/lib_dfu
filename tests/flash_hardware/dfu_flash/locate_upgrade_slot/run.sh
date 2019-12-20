#!/bin/sh
echo locate_upgrade_slot

# expected 28672 = 20,480 bytes for flattened hello world app + stage 2 loader, then round to whole 4KB sectors

# case 1: upgrade absent
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe || exit $?
xrun --io --args bin/locate_upgrade_slot.xe 28672

# case 2: upgrade present
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe --upgrade 1 ../hello_world.xe || exit $?
xrun --io --args bin/locate_upgrade_slot.xe 28672
