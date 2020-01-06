Boot logic tests validated by reading a memory location using gdb

Shell script in each test invokes the appropriate xflash command to program
flash with loader, then it checks for result

To run all tests, I would do:

  make -C ../../host/data_partition_generator
  export PATH=$PATH:$PWD/../../host/data_partition_generator/bin

  make -C ../../boot headers

  time ( ls -d * | grep -v helpers | while read t ; do if [ -d $t ] ; then \
    ( cd $t ; sh run.sh ) || break ; fi ; done )

