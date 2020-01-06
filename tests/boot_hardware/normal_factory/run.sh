#!/bin/sh

make -C ../../../boot headers

# boot factory and data factory images are present and compatible

# data upgrade image either absent, present or present but bad
data_partition_generator --regular-sector-size 4096 -o /tmp/data1.bin --factory factory.json || exit $?
data_partition_generator --regular-sector-size 4096 -o /tmp/data2.bin --factory factory.json --upgrade 514 upgrade.json || exit $?
data_partition_generator --regular-sector-size 4096 -o /tmp/data3.bin --factory factory.json --bad-upgrade-crc --upgrade 514 upgrade.json || exit $?

# situation A: boot upgrade absent
for data in 1 2 3 ; do
  break
  xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../helpers/cafecafe_60000.xe --data /tmp/data$data.bin --loader ../../../boot/loader.c || exit $?
  xgdb -quiet -batch -ex att -ex 'x 0x60000' -ex det | grep 0x60000 | cut -d: -f2
done

# situation B: boot upgrade present but bad
for data in 1 2 3 ; do
  xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../helpers/cafecafe_60000.xe --upgrade 514 ../helpers/hello_world.xe --data /tmp/data$data.bin --loader ../../../boot/loader.c || exit $?
  xrun --io ../helpers/corrupt_boot_upgrade_image.xe || exit $?
  xgdb -quiet -batch -ex 'conn --reset-to-mode-pins' -ex det -ex att -ex 'x 0x60000' -ex det | grep 0x60000 | cut -d: -f2
done
