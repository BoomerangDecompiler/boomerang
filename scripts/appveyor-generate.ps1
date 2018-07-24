
$QT_VERSION="5.9"

if ($env:APPVEYOR_BUILD_WORKER_IMAGE -eq "Visual Studio 2017") {
    $CMAKE_GENERATOR_NAME = "Visual Studio 15 2017 Win64"
    $QT_BASE_DIR = "C:\Qt\$QT_VERSION\msvc2017_64"
}


# install libcapstone via vcpkg
vcpkg install capstone


$QT_CORE_DIR    = "$QT_BASE_DIR\lib\cmake\Qt5Core"
$QT_GUI_DIR     = "$QT_BASE_DIR\lib\cmake\Qt5Gui"
$QT_TEST_DIR    = "$QT_BASE_DIR\lib\cmake\Qt5Test"
$QT_WIDGETS_DIR = "$QT_BASE_DIR\lib\cmake\Qt5Widgets"
$QT_XML_DIR     = "$QT_BASE_DIR\lib\cmake\Qt5Xml"

$CMAKE_PREFIX_PATH="$QT_CORE_DIR;$QT_GUI_DIR;$QT_TEST_DIR;$QT_WIDGETS_DIR;$QT_XML_DIR"

# Build Visual Studio solution
if ($env:CONFIGURATION -eq "Debug") {
    $SHARED_LIBS = "OFF"
}
else {
    $SHARED_LIBS = "ON"
}

cmake -G "$CMAKE_GENERATOR_NAME" -DCMAKE_TOOLCHAIN_FILE="c:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" -DBOOMERANG_BUILD_UNIT_TESTS=ON -DBUILD_SHARED_LIBS="$SHARED_LIBS" ..
