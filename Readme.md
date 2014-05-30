This repository is now connected to continous integration server:
http://boomerang.nemerle.eu

This is an experimental branch of boomerang project, the inital goals are:

1. try to fix all warnings from gcc -Wall, and clang -Wall
1. verify stability of decompilation results : compare 2 run results, they should be exactly the same.
1. simplify code base using features available in c++11

Considering pt. 3, the target compilers are gcc >=4.6 family (mingw on windows), and clang.

For full ( and slightly stale :) ) build instructions please see

        http://boomerang.sourceforge.net/making.html

Note that on a debian system you can usually get away with:
```bash
  sudo apt-get install libgc-dev
  sudo apt-get install libexpat-dev
  sudo apt-get install git
  sudo apt-get install cmake
  cd YOUR_FAVOURITE_DEVELOPMENT_DIRECTORY
  git clone git://github.com/nemerle/boomerang.git
  cd boomerang
  mkdir build && cd build
  cmake ..
  make
```
To compile the qt5_and_boost branch on windows, it should be enough to have:
* cmake 2.8.12 or newer from http://www.cmake.org/cmake/resources/software.html
* Qt5 mingw package ( version 5.3 or newer ) available from http://www.cmake.org/cmake/resources/software.html


After building boomerang You can run the test suite, to do that you will need ruby interpeter and a bash compatible shell.
```
  ./full_regression.sh
```
After running full_regression, the tool will report tests on which boomerang crashed.
You can also check if Your changes to boomerang, produced any changes in the quality of decompiled code by running
  YOUR_FAVOURITE_DIFF_GUI ./tests/outputs ./tests/baseline
Thanks.

