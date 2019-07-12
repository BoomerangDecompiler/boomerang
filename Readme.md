# Boomerang Decompiler

This is a fork of the [Boomerang Decompiler](http://boomerang.sourceforge.net/), a general, open source (BSD licensed) machine code decompiler.
Boomerang currently supports:
 - architectures: x86 (IA-32 only), SPARC (V8/V9), PPC, ST20
 - file formats: ELF, PE, DOS MZ, DOS/4GW LE, Mach-O
 - high-level language output: C

Although there are pre-compiled packages available for release versions (`master` branch),
it is currently recommended to build the development version (`develop` branch) of the decompiler from source.


## Building

| **Build status** | Linux/macOS | Windows | Test Coverage |
|------------------|-------------|---------|---------------|
|    **develop**   | [![Travis CI](https://api.travis-ci.com/BoomerangDecompiler/boomerang.svg?branch=develop)](https://travis-ci.com/BoomerangDecompiler/boomerang/branches) | [![Appveyor](https://ci.appveyor.com/api/projects/status/pg2bw7kxse1t7cx8/branch/develop?svg=true)](https://ci.appveyor.com/project/ceeac/boomerang/branch/develop) | [![codecov](https://codecov.io/gh/BoomerangDecompiler/boomerang/branch/develop/graph/badge.svg)](https://codecov.io/gh/BoomerangDecompiler/boomerang/branch/develop) |
|    **master**    | [![Travis CI](https://api.travis-ci.com/BoomerangDecompiler/boomerang.svg?branch=master)](https://travis-ci.com/BoomerangDecompiler/boomerang/branches)  | [![Appveyor](https://ci.appveyor.com/api/projects/status/pg2bw7kxse1t7cx8/branch/master?svg=true)](https://ci.appveyor.com/project/ceeac/boomerang/branch/master)   | [![codecov](https://codecov.io/gh/BoomerangDecompiler/boomerang/branch/master/graph/badge.svg)](https://codecov.io/gh/BoomerangDecompiler/boomerang/branch/master)   |


### Building prerequisites

 - A 64 bit operating system (32 bit might or might not work, but it is not supported.)
 - A C++17 compiler (GCC 7+, Clang 6+, MSVC 2017+ are known to work)
 - [CMake](https://cmake.org/download/) 3.13+
 - [Qt5](https://www.qt.io/download-open-source/) (Qt 5.12+ is known to work, earlier versions should also work)
 - [Capstone](http://www.capstone-engine.org/) 4.0.1+
 - [GNU bison](https://www.gnu.org/software/bison/) 3.3+
 - [GNU flex](https://github.com/westes/flex) 2.6+
 - [CCache](https://ccache.samba.org/download.html) 3.2+ (optional, for recompilation speed)
 - [Doxygen](http://www.doxygen.nl/) 1.8+ (optional, for documentation)
 - [Python 3](https://www.python.org/downloads/) (optional, for regression tests)


### Building on Linux

On a Linux system you can build and install Boomerang with the usual cmake-make-make-install procedure.
On a Debian-compatible system (e.g. Ubuntu) these commands will clone, build and install Boomerang:

```bash
sudo apt-get install git build-essential cmake qt5-default libcapstone-dev flex bison
cd YOUR_FAVOURITE_DEVELOPMENT_DIRECTORY
git clone https://github.com/BoomerangDecompiler/boomerang.git
cd boomerang && mkdir build && cd build
cmake .. && make -j$(nproc) && sudo make install
```

### Building on macOS

To build Boomerang on macOS, you need at least macOS 10.13 or later, and XCode 10 or later.
The recommended way of installing Boomerang and its dependencies outlined below is via [Homebrew](brew.sh), although other methods might also work (untested).
After installing XCode, execute the following commands in a terminal window:

```bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" # Install Homebrew
brew install git cmake qt capstone flex bison # Install dependencies
cd YOUR_FAVOURITE_DEVELOPMENT_DIRECTORY
git clone https://github.com/BoomerangDecompiler/boomerang.git
cd boomerang && mkdir build && cd build
cmake -DQt5_DIR=$(brew --prefix qt5)/lib/cmake/Qt5/ -DFLEX_EXECUTABLE=$(brew --prefix flex)/bin/flex -DBISON_EXECUTABLE=$(brew --prefix bison)/bin/bison ..
make -j$(sysctl -n hw.ncpu) && make install
```

### Building on Windows

To compile on Windows using Visual Studio 2017, you can follow the following guide. Note that the build procedure
for other IDEs or compilers (e.g. MinGW) is not covered in this guide.

- Install Visual Studio 2017 (e.g the free [Community Edition](https://visualstudio.microsoft.com/vs/community/)).
- Install [Git for Windows](https://github.com/git-for-windows/git/releases/latest).
- Install [CMake](https://cmake.org/download/).
- Install flex+bison via [winflexbison](https://github.com/lexxmark/winflexbison).
- Download and install [Qt5](https://www.qt.io/download-open-source/). Please make sure to install the 64-bit Windows version for Visual Studio 2017.
- Set the QTDIR environment variable. For example, if you have installed Qt 5.10.0 into C:\Qt, set QTDIR to "C:\Qt\5.10.0\msvc2017_64\" (without the quotes).
- Clone Boomerang using Git for Windows. Let's call the directory of the cloned repository $BOOMERANG_DIR.
- Open cmake-gui and enter $BOOMERANG_DIR and $BOOMERANG_DIR/build into the "Where is the source code" and "Where to build the binaries" fields, respectively.
- "Configure" Boomerang in cmake-gui. Make sure to select the "Visual Studio 15 2017 Win64" (i.e. 64-bit) generator.
- "Generate" and "Open Project" in cmake-gui.
- To build the command line tool, build the `boomerang-cli` target; to build the GUI, build the `boomerang-gui` target.
- Done!


## Usage

Boomerang can be run in two ways:

- `boomerang-cli[.exe]`: Command-line interface. Use `boomerang-cli[.exe] --help` for help wrt. how to customize the decompilation process.
- `boomerang-gui[.exe]`: GUI application.

There are a number of test applications available in the `data/samples` directory. You can use these to assess
what the decompiler can decompile and also to experiment with the different decompilation options.


## Testing

### Unit tests

Boomerang has a unit test suite, which can be run by `make && make test` on Linux or by running the RUN_TESTS target in Visual Studio.
Make sure you have the BOOMERANG_BUILD_UNIT_TESTS option set in CMake.


### Regression tests

Additionally, you can run the regression test suite, to do so you will need a Python 3 interpeter.
To run the regression test suite, make sure the BOOMERANG_BUILD_REGRESSION_TESTS option is set in CMake, then run `make check`
on Linux. Building the regression test suite on Windows is currently not supported.

When the regression test suite finds a regression in the output, it is shown as a unified diff.
If you have not modified Boomerang, please file the regression(s) as a bug report at https://github.com/BoomerangDecompiler/boomerang/issues.


# Contributing

Boomerang uses the [gitflow workflow](https://nvie.com/posts/a-successful-git-branching-model/). If you want to fix a bug or implement a small enhancement,
please branch off from the `develop` branch (`git checkout -b`) and submit your fix or enhancement as a pull request to the `develop` branch.
If you want to implement a larger feature, please open an issue about the new feature on the issue tracker first, so the feature can be discussed first.
For additional information, please read the [contributing guidelines](https://github.com/BoomerangDecompiler/boomerang/blob/develop/Contributing.md).

Thanks for your interest in the Boomerang Decompiler!
