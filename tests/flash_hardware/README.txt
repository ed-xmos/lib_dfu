Tests of device-side extended flash functionality

These run on hardware and so are manually executed rather than using Pytest

Normally there will be a shell script with each test that shows how to run it

Convention is that pass is when last line of test output is 'PASS'

To run all tests I might do:

  make -C ../../host/data_partition_generator
  export PATH=$PATH:$PWD/../../host/data_partition_generator

  time ( ls -d */* | while read t ; do if [ -d $t ] ; then \
    ( cd $t ; waf configure clean build >/dev/null && \
      sh run.sh ) || break ; fi ; done )
