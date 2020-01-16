Unit tests of device-side core DFU functionality

Pytest evaluates return code of the simulator process, which will be 0 on
success and non-zero if test failed (typically it is one of the test assertion
that fails).

Convention is that pass is when last line of test output is 'PASS'

Build is using Waf with a customised top level script that traverses test
subdirectories and builds each test

To run all tests in Pipenv I might do:

  waf configure clean build
  pipenv run pytest -s

I've also added a __main__ trigger, so I can invoke the test script outside of
Pipenv:

  waf configure clean build
  find . -name test_\*.py | while read f ; do \
    ( cd `dirname $f` ; python `basename $f` ) || break ; done
