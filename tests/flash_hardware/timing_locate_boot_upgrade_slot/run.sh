#!/bin/sh
echo timing_locate_boot_upgrade_slot

# factory only
# observed 60ms with the approximately 28KB hello world, so set threshold at 100ms
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe || exit $?
xrun --io --args bin/timing_locate_boot_upgrade_slot.xe 10000000

# both factory and upgrade
# observed 515us with the approximately 28KB hello world, so set threshold at 1ms
xflash --no-compression --noinq --quad-spi-clock=12.5MHz --factory ../hello_world.xe --upgrade 1 ../hello_world.xe || exit $?
xrun --io --args bin/timing_locate_boot_upgrade_slot.xe 100000
