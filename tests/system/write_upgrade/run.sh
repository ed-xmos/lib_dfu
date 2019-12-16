#!/bin/sh
for block_size in 32 128 256 512 ; do
  xflash --no-compression --noinq --quad-spi-clock=12.5MHz \
    --factory ../hello_world.xe 
  xrun --io --args bin/write_upgrade.xe ../hello_world.bin $block_size
done
