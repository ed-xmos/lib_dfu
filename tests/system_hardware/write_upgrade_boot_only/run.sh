#!/bin/sh
# upgrade address 28672 = 8KB stage 2 loader + 20,480 bytes hello world image
for block_size in 32 128 256 512 ; do
  xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe || exit $?
  xrun --io --args bin/write_upgrade_boot_only.xe ../hello_world.bin $block_size 28672 || exit $?
done
