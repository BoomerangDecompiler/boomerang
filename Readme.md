Welcome to boomerang decompiler
===============================

This repository is now connected to continous integration server:
<http://boomerang.nemerle.eu>
Generated documentation is available at <http://boomerang.nemerle.eu/job/Boomerang-next/doxygen/>

The pre-compiled windows version is available at <http://nemerle.eu/boomerang.zip>

This is an experimental branch of boomerang project, the inital goals are:

1. Try to fix all warnings from GCC and Clang's `-Wall`
2. Verify stability of decompilation results: compare 2 run results, they should be exactly the same.
3. Simplify code base using features available in c++11

Considering part 3, the target compilers are GCC \>=4.6 family (MinGW on windows), and Clang.

For full ( and slightly stale :) ) build instructions please see

<http://boomerang.sourceforge.net/making.html>
----------------------------------------------

Compiling the `next` branch
===========================

Note that on a debian system you can usually get away with:

```bash
sudo apt-get install git libboost-dev cmake qt5-default
cd YOUR_FAVOURITE_DEVELOPMENT_DIRECTORY
git clone git://github.com/nemerle/boomerang.git
cd boomerang
mkdir build && cd build
cmake ..
make
```

To compile on windows, it should be enough to have:

-   [CMake 2.8.12 or newer](http://www.CMake.org/CMake/resources/software.html)
-   [Qt5 for MinGW](http://qt-project.org/wiki/MinGW-64-bit)
-   QtCreator (installed with Qt5)

Testing
=======

After building boomerang You can run the test suite, to do that you will need ruby interpeter and a bash compatible shell.

      ./full_regression.sh

After running full\_regression, the tool will report tests on which boomerang crashed.
You can also check if Your changes to boomerang, produced any changes in the quality of decompiled code by running
 YOUR\_FAVOURITE\_DIFF\_GUI ./tests/outputs ./tests/baseline

Additionally, if You enable the test suite option in ( CMake option ), boomerang unit-test can be run by

    make test

Thanks.
