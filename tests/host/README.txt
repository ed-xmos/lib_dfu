Host tests to run on UNIX platforms

First build host apps

  cd lib_dfu/host
  cmake -B build
  cmake --build build

Then move to test folder and build tests

  cd lib_dfu/tests/host
  cmake -B build
  cmake --build build

Run tests

  cd lib_dfu/tests
  pytest -k host

Alternatively directly from hosts folder

  cd lib_dfu/tests/host
  pytest
