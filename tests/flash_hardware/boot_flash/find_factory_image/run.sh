#!/bin/sh
# note: typically a boot factory image starts outside of a page or sector boundary
echo find_factory_image
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../../hello_world.xe || exit $?
xrun --io --args bin/find_factory_image.xe 5672
