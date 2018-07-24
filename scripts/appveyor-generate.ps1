
$QT_VERSION="5.11"

if ($env:APPVEYOR_BUILD_WORKER_IMAGE -eq "Visual Studio 2017") {
    $CMAKE_GENERATOR_NAME = "Visual Studio 15 2017 Win64"
    $QT_BASE_DIR = "C:\Qt\$QT_VERSION\msvc2017_64"
}

$env:QTDIR = "$QT_BASE_DIR"

# install libcapstone via vcpkg
vcpkg install capstone:x64-windows


# Build Visual Studio solution
if ($env:CONFIGURATION -eq "Debug") {
    $SHARED_LIBS = "OFF"
}
else {
    $SHARED_LIBS = "ON"
}

# Build Visual Studio solution
cmake -G "$CMAKE_GENERATOR_NAME" \
    -DCMAKE_TOOLCHAIN_FILE="c:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" \
    -DBOOMERANG_BUILD_UNIT_TESTS=ON 
    -DBUILD_SHARED_LIBS="$SHARED_LIBS" ..

