# Welcome to boomerang decompiler

This repository is now connected to continous integration server:
http://boomerang.nemerle.eu
Generated documentation is available at http://boomerang.nemerle.eu/job/Boomerang-next/doxygen/
The pre-compiled windows version is available at http://nemerle.eu/boomerang.zip

This is an experimental branch of boomerang project, the inital goals are:

1. try to fix all warnings from gcc -Wall, and clang -Wall
1. verify stability of decompilation results : compare 2 run results, they should be exactly the same.
1. simplify code base using features available in c++11

Considering pt. 3, the target compilers are gcc >=4.6 family (mingw on windows), and clang.

For full ( and slightly stale :) ) build instructions please see

        http://boomerang.sourceforge.net/making.html
--------------------------------------
#  Compiling the 'next' branch

Note that on a debian system you can usually get away with:
```bash
  sudo apt-get install git
  sudo apt-get install libboost-dev
  sudo apt-get install cmake
  sudo apt-get install qt5-default
  cd YOUR_FAVOURITE_DEVELOPMENT_DIRECTORY
  git clone git://github.com/nemerle/boomerang.git
  cd boomerang
  mkdir build && cd build
  cmake ..
  make
```
To compile on windows, it should be enough to have:
* cmake 2.8.12 or newer from http://www.cmake.org/cmake/resources/software.html
* Qt5 mingw package ( version 5.3 or newer ) available from http://www.cmake.org/cmake/resources/software.html
* QtCreator - installed as a part of the Qt5.

# Testing

After building boomerang You can run the test suite, to do that you will need ruby interpeter and a bash compatible shell.
```
  ./full_regression.sh
```
After running full_regression, the tool will report tests on which boomerang crashed.
You can also check if Your changes to boomerang, produced any changes in the quality of decompiled code by running
  YOUR_FAVOURITE_DIFF_GUI ./tests/outputs ./tests/baseline

Additionally, if You enable the test suite option in ( cmake option ), boomerang unit-test can be run by
```
make test
```

Thanks.

