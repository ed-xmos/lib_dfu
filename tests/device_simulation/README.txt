Unit tests of device-side core DFU functionality

Pytest evaluates return code of the simulator process, which will be 0 on
success and non-zero if test failed (typically it is one of the test assertion
that fails).

Convention is that pass is when last line of test output is 'PASS'

To run all tests in Pipenv I might do:

  time ( ls -d buffer_converter dfu/* | \
    while read t ; do if [ -d $t ] ; then \
      ( cd $t ; waf configure clean build >/dev/null && cd - ; cd ../.. ; \
        pipenv run pytest tests/device_simulation/$t -s ) || break ; fi ; done )

I've also added a __main__ trigger, so I can invoke the test script outside of
Pipenv:

  time ( ls -d buffer_converter dfu/* quadflash_data_partition/* | \
    while read t ; do if [ -d $t ] ; then \
      ( cd $t ; waf configure clean build >/dev/null && \
        python test_`basename $t`.py ) || break ; fi ; done )
