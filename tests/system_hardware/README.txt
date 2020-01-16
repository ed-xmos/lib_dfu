System tests of entire device-side operations such as write upgrade

These run on hardware and so are manually executed rather than using Pytest

Normally there will be a shell script with each test that shows how to run it

Build is using Waf with a customised top level script that traverses test
subdirectories and builds each test, eg

  waf configure clean build
