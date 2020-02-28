#!/bin/sh
echo timing_locate_data_upgrade_slot

python make_test_json.py /tmp/factory.json 2.0.1 262144
python make_test_json.py /tmp/upgrade.json 2.0.2 262144

# factory only
# observed 133ms with the 256KB data image so set threshold at 200ms
data_partition_generator --hardware-build 0x12345678 --spi-spec-bin spispec.bin --regular-sector-size 4096 --factory /tmp/factory.json -o /tmp/data.bin || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/timing_locate_data_upgrade_slot.xe 20000000

# both factory and upgrade
# observed 265ms with the 256KB data image so set threshold at 300ms
data_partition_generator --hardware-build 0x12345678 --spi-spec-bin spispec.bin --regular-sector-size 4096 --factory /tmp/factory.json -o /tmp/data.bin --upgrade 0x202 /tmp/upgrade.json || exit $?
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --boot-partition-size 1048576 --factory ../hello_world.xe --data /tmp/data.bin || exit $?
xrun --io --args bin/timing_locate_data_upgrade_slot.xe 30000000
