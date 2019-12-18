#!/bin/sh
echo get_next_data_image

data_partition_generator --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/get_next_data_image.xe -

data_partition_generator --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin --upgrade 514 upgrade.json || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/get_next_data_image.xe 1048576 20 514
