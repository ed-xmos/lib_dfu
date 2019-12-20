#!/bin/sh
echo get_next_data_image

# start address 1056768 = 1MB data partition base + 4KB for sector-padded hardware build section + 4KB for sector-padded factory data image
# 24 bytes of size for 2 words of TLV data makes for a 24-byte image

# case 1: factory data image absent
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe || exit $?
xrun --io bin/get_next_data_image.xe

# case 2: upgrade data image absent
data_partition_generator --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/get_next_data_image.xe F

# case 3: upgrade data image present
data_partition_generator --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin --upgrade 514 upgrade.json || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/get_next_data_image.xe 1056768 24 514
