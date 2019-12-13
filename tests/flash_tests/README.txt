These run on hardware and so are manually executed rather than using Pytest

Convention is that pass is when last line of test output is 'PASS'

To run all tests I might do:

  time ( ls | while read t ; do if [ -d $t ] ; then \
    ( cd $t ; waf configure clean build && sh run.sh ) || break ; fi ; done )
