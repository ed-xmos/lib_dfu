#!/bin/sh
echo get_factory_data_image_bad_crc
data_partition_generator --bad-factory-crc --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io bin/get_factory_data_image_bad_crc.xe
