#!/bin/sh
echo locate_data_upgrade_slot

# expected 1056768 = 1MB data partition base + 4KB for sector-padded hardware build section + 4KB for sector-padded factory data image

# data upgrade absent
data_partition_generator --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/locate_data_upgrade_slot.xe 1056768

# data upgrade present but bad CRC
data_partition_generator --bad-upgrade-crc --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin --upgrade 514 upgrade.json || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/locate_data_upgrade_slot.xe 1056768

# data upgrade present
data_partition_generator --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin --upgrade 514 upgrade.json || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/locate_data_upgrade_slot.xe 1056768
