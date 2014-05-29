This repository is now connected to continous integration server:
http://boomerang.nemerle.eu

This is an experimental branch of boomerang project, the inital goals are:
1. try to fix all warnings from gcc -Wall, and clang -Wall
2. verify stability of decompilation results : compare 2 run results, they should be exactly the same.
3. simplify code base using features available in c++11 

Considering pt. 3, the target compilers are gcc >=4.6 family (mingw on windows), and clang

For full ( and slightly stale :) ) build instructions please see

        http://boomerang.sourceforge.net/making.html

Note that on a debian system you can usually get away with:
  sudo apt-get install libgc-dev
  sudo apt-get install libexpat-dev
  cd YOUR_FAVOURITE_DEVELOPMENT_DIRECTORY
  git clone git://github.com/nemerle/boomerang.git
  cd boomerang
  mkdir build && cd build
  cmake ..
  make

After building boomerang You can run the test suite, to do that you will need ruby interpeter and a bash compatible shell.
  ./full_regression.sh
After runnign full_regression, the tool will report tests on which boomerang crashed. 
You can also check if Your changes to boomerang, produced any changes in the quality of decompiled code by running
  YOUR_FAVOURITE_DIFF_GUI ./tests/outputs ./tests/baseline
Thanks.

