Host tests to run on UNIX platforms

Manual Pytest wrappers that return 0 on success and non-zero if test failed

To run all tests in Pipenv I might do:

  time ( ls -d */* | while read t ; do if [ -d $t ] ; then \
    ( cd $t ; make clean all >/dev/null && cd - ; cd ../.. ; \
      pipenv run pytest tests/host/$t -s ) || break ; fi ; done )

I've also added a __main__ trigger, so I can invoke the test script outside of
Pipenv:

  time ( ls -d */* | while read t ; do if [ -d $t ] ; then \
    ( cd $t ; make clean all >/dev/null && \
      python test_`basename $t`.py ) || break ; fi ; done )
