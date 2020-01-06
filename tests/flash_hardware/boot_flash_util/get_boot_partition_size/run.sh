#!/bin/sh
echo get_boot_partition_size

# no data partition allocated
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../../hello_world.xe || exit $?
xrun --io --args bin/get_boot_partition_size.xe 2097152

# data upgrade allocated
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe || exit $?
xrun --io --args bin/get_boot_partition_size.xe 1048576
