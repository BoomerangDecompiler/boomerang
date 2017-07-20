# Boomerang Decompiler

This is an experimental fork of the [Boomerang Decompiler](http://boomerang.sourceforge.net/).
Right now, there are no pre-compiled binaries available, so you'll have to compile from source yourself.

## Building

### Building prerequisites

 - A C++11 compatible compiler (GCC \>= 5.0, Clang \>= 4.0, MSVC \>= 2015 are known to work)
 - [CMake 3.1.0 or newer](https://cmake.org/download/)
 - [Qt5](https://www.qt.io/download-open-source/)
 - [Doxygen](http://www.doxygen.nl/) (optional)

### Building on Linux

Note that on a Debian-compatible system you can usually get away with:

```bash
sudo apt-get install git cmake qt5-default
cd YOUR_FAVOURITE_DEVELOPMENT_DIRECTORY
git clone git://github.com/ceeac/boomerang.git
cd boomerang
mkdir build && cd build
cmake ..
make
make install
```

### Building on Windows

To compile on windows, it should be enough to run cmake-gui and fill in the paths to missing libraries if CMake des not find them.


### Building on macOS

Building on macOS is currently not supported. However, patches or pull requests on this matter are welcome.


## Testing

### Unit tests

Boomerang has a unit test suite, which cah be run by `make test` on Linux or by running the RUN_TESTS target in Visual Studio.

### Regression tests

Additionally, you can run the regression test suite, to do so you will need a Ruby or Python interpeter and a bash compatible shell.
To run the regression test suite, run `./full_regression.sh`.

After running the test suite, the tool will report tests on which boomerang crashed.
You can also check if your changes to boomerang produced any changes in the quality of decompiled code by running
 YOUR\_FAVOURITE\_DIFF\_GUI ./tests/outputs ./tests/baseline



