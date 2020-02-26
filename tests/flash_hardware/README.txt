Tests of device-side flash-related functionality

These run on hardware and so are manually executed rather than using Pytest
Build is using Waf with a customised top level script that traverses test
subdirectories and builds each test

Normally there will be a shell script with each test that shows how to run it

Convention is that pass is when last line of test output is 'PASS'

The data partition generator must be compiled and reachable in system path in
order to run tests

Build is using Waf with a customised top level script that traverses test
subdirectories and builds each test

To build and run all tests I might do:

  waf configure clean build

  find . -name run.sh | while read f ; do \
    ( cd `dirname $f` ; sh -x run.sh ) || break ; done

Note that some of these will require the Data Partition Generator utility from
Flash Data Partition library
