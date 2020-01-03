#!/bin/sh
echo find_upgrade_image
# expected 28672 = 20,480 bytes for flattened hello world app + stage 2 loader, then round to whole 4KB sectors
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../../hello_world.xe --upgrade 1 ../../hello_world.xe || exit $?
xrun --io --args bin/find_upgrade_image.xe 28672
