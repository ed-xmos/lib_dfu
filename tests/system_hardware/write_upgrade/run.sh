#!/bin/sh
# upgrade address 28672 = 8KB stage 2 loader + 20,480 bytes hello world image
# upgrade address 1056768 = 1MB data partition base + 4KB for sector-padded hardware build section + 4KB for sector-padded factory data image
data_partition_generator --regular-sector-size 4096 --hardware-build 0x12345678 -o /tmp/factory.bin --factory factory.json || exit $?
data_partition_generator --regular-sector-size 4096 -o /tmp/upgrade.bin --upgrade 514 upgrade.json || exit $?
for block_size in 32 128 256 512 ; do
  xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../hello_world.xe --data /tmp/factory.bin || exit $?
  xrun --io --args bin/write_upgrade.xe ../hello_world.bin /tmp/upgrade.bin $block_size 28672 1056768 || exit $?
done
