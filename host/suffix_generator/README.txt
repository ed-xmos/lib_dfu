DFU Suffix Generator
====================

Typical usage
-------------

Input (image.bin below) is a boot image produced by xflash or a data partition
image produced by the appropriate generator utility (Flash data partition
library). Output (final.bin below) is a version of the same with DFU suffix
appended.

    suffix_generator 0x20B1 0x0014 image.bin final.bin

BCD device
----------

Only vendor ID and product ID are specified. The BCD device field is set to
0xFFFF, meaning 'do not check'. This gives the ability to upgrade multiple
versions of running firmware to the new one, and is the more desired use case
than upgrading one specific running version.
