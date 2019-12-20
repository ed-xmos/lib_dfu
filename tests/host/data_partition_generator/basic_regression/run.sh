#!/bin/sh
echo basic_regression

data_partition_generator --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin || exit $?
diff /tmp/data.bin golden1.bin || exit $?

data_partition_generator --regular-sector-size 4096 --factory factory.json --upgrade 514 upgrade.json -o /tmp/data.bin || exit $?
diff /tmp/data.bin golden2.bin || exit $?

data_partition_generator --bad-factory-crc --regular-sector-size 4096 --factory factory.json -o /tmp/data.bin || exit $?
diff /tmp/data.bin golden3.bin || exit $?

data_partition_generator --bad-upgrade-crc --regular-sector-size 4096 --factory factory.json --upgrade 514 upgrade.json -o /tmp/data.bin || exit $?
diff /tmp/data.bin golden4.bin || exit $?

echo PASS
