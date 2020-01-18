DFU Suffix Generator
====================

Typical usage
-------------

Input is a boot image produced by xflash or a data partition image produced by
the appropriate generator utility (Flash data partition library). Ouptut is a
version of the same with DFU suffix appended.

    suffix_generator 0x20B1 0x0014 0x0210 < image.bin > final.bin
