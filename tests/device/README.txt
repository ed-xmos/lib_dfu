Unit tests of device-side core DFU functionality

Pytest evaluates return code of the simulator process, which will be 0 on
success and non-zero if test failed (typically it is one of the test assertion
that fails).

Convention is that pass is when last line of test output is 'PASS'

To run all tests I might do:

  time ( ls | while read t ; do if [ -d $t ] ; then \
    ( cd $t ; waf configure clean build && cd ../../.. ; \
      pipenv run pytest tests/device/$t -s ) || break ; fi ; done )
