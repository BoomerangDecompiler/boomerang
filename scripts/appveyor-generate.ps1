
$QT_VERSION="5.11"

if ($env:APPVEYOR_BUILD_WORKER_IMAGE -eq "Visual Studio 2017") {
    $CMAKE_GENERATOR_NAME = "Visual Studio 15 2017 Win64"
    $QT_BASE_DIR = "C:\\Qt\\$QT_VERSION\\msvc2017_64\\"
}

$env:QTDIR = "$QT_BASE_DIR"

# Install flex + bison via winflexbison
if (!(Test-Path winflexbison.zip)) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    if (Invoke-WebRequest -Uri https://github.com/lexxmark/winflexbison/releases/download/v2.5.18/win_flex_bison-2.5.18.zip -OutFile winflexbison.zip) {
        Write-Output "Could not download winflexbison"
        exit 1
    }
}

$expectedhash = "095CF65CB3F12EE5888022F93109ACBE6264E5F18F6FFCE0BDA77FEB31B65BD8"
$actualhash = (Get-FileHash -Algorithm "SHA256" winflexbison.zip).hash

if ($actualhash -ne $expectedhash) {
    Write-Output "File hash does not match: Expected: $expectedhash, Actual: $actualhash"
    exit 1
}

if (!(Test-Path winflexbison)) {
    Expand-Archive -LiteralPath winflexbison.zip -DestinationPath winflexbison
}

# install capstone via vcpkg
vcpkg install capstone[core,sparc,x86,ppc]:x64-windows


# Build Visual Studio solution
cmake -G "$CMAKE_GENERATOR_NAME" `
    -DCMAKE_TOOLCHAIN_FILE="c:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake" `
    -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" `
    -DBOOMERANG_BUILD_UNIT_TESTS=ON `
    -DBISON_EXECUTABLE="C:/projects/boomerang/build/winflexbison/win_bison.exe" `
    -DFLEX_EXECUTABLE="C:/projects/boomerang/build/winflexbison/win_flex.exe" ..
