DFU library
===========

.. rheader::

   DFU library |version|

DFU library
-----------

Device firmware upgrade but also flash data partition support and boot

Features
........

  - One factory and one upgrade boot slots

Components
...........

Data partition support
----------------------

Definitions to describe structure of flash data partition and data images.

Includes extensions to the tools-provided ``quadflash`` library so that it
understands data partition, can read from it and write to it.

Host utility for generation of data images. These are typically passed to
xflash for flash programming.

Deliberately not a library of its own, because it is extending the tools
libraries and we might choose to use 

DFU
---

Device and host code for requests such as *get status* and *download* that
implement firmware upgrade with data partition support.

Boot logic
----------

Extensions to the tools-provided ``sqiaccess`` and ``quadspi`` libraries
so that they can access data partition.

These are used by the xflash custom loader to implement behaviour in various
boot scenarios such as valid factory images but invalid data upgrade image
(fall back to factory in that case).

Header-only C modules since the boot loader is a single C file passed to
xflash.

Software version and dependencies
.................................

.. libdeps::

Related application notes
.........................

None
