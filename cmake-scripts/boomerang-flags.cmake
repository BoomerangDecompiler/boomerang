#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


# Check for required compiler version
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
        message(FATAL_ERROR "Your Clang version is too old.\n\
            Please upgrade Clang or select another compiler.")
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
        message(FATAL_ERROR "Your GCC version is too old.\n\
            Please upgrade GCC or select another compiler.")
    endif ()
endif ()


# Force C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Hide all symbols unless explicitly exported
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(COMMON_COMPILE_FLAGS "")
set(C_COMPILE_FLAGS "")
set(CXX_COMPILE_FLAGS "")
set(LINKER_FLAGS "")

include(boomerang-utils)

# Add compiler flags if available
if (MSVC)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS
    )

    BOOMERANG_ADD_COMPILE_FLAGS(/W4 /WX)
    BOOMERANG_ADD_COMPILE_FLAGS(/EHsc /MP /nologo)
    BOOMERANG_ADD_COMPILE_FLAGS(/wd4267) # conversion from size_t to int
    BOOMERANG_ADD_COMPILE_FLAGS(/wd4091) # 'typedef ': ignored on left of '' when no variable is declared
    BOOMERANG_ADD_COMPILE_FLAGS(/wd4702) # Unreachable code
    BOOMERANG_ADD_COMPILE_FLAGS(/wd4127) # conditional expression is constant
    BOOMERANG_ADD_COMPILE_FLAGS(/wd4251) # Class needs to have DLL interface to be used by clients
    BOOMERANG_ADD_COMPILE_FLAGS(/wd4005) # Macro redifinition
    BOOMERANG_ADD_COMPILE_FLAGS(/wd4065) # switch statement only contains default but no case labels

    set(DEBUG_LIB dbghelp.lib)

else () # GCC / Clang
    BOOMERANG_ADD_COMPILE_FLAGS(-Wall -Wextra -Werror -Werror=pedantic)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wshadow)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wformat=2)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wmissing-include-dirs)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wstrict-overflow=2)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wnull-dereference)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wduplicated-cond -Wduplicated-branches)
    BOOMERANG_ADD_COMPILE_FLAGS(-Walloc-zero -Walloca)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wsuggest-override)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wundef)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wcast-qual -Wfloat-conversion -Wdouble-promotion)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wmissing-variable-declarations)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wno-unknown-pragmas) # pragma region is not supported by GCC
    BOOMERANG_ADD_COMPILE_FLAGS(-fno-strict-aliasing) # Will break *reinterpret_cast<float*>(&int) otherwise
    BOOMERANG_ADD_COMPILE_FLAGS(-Wno-gnu-zero-variadic-macro-arguments) # Will break QSKIP() macro on clang otherwise

    # Do not treat specific warnings as errors
    BOOMERANG_ADD_COMPILE_FLAGS(-Wno-error=strict-overflow)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wno-error=alloca)
    BOOMERANG_ADD_COMPILE_FLAGS(-Wno-error=implicit-fallthrough)

    # Special workarounds for bugs in dependencies

    if (Qt5Core_VERSION VERSION_GREATER 5.6.1)
        # See https://bugreports.qt.io/browse/QTBUG-45291
        BOOMERANG_ADD_COMPILE_FLAGS(-Wzero-as-null-pointer-constant)
    endif ()

    if (BOOMERANG_ENABLE_CCACHE AND CCache_VERSION VERSION_LESS 3.2.0 AND CMAKE_C_COMPILER MATCHES "[Cc]lang")
        # see https://bugzilla.samba.org/show_bug.cgi?id=8118
        BOOMERANG_ADD_COMPILE_FLAGS(
            -Wno-unused-command-line-argument
            -Wno-error=inconsistent-missing-override
        )
    endif ()

    BOOMERANG_ADD_COMPILE_FLAGS(-Wno-error=zero-as-null-pointer-constant)

    # Other warnings
#    BOOMERANG_ADD_COMPILE_FLAGS(-Wcast-qual)
#    BOOMERANG_ADD_COMPILE_FLAGS(-Wconversion)
#    BOOMERANG_ADD_COMPILE_FLAGS(-Wswitch-enum)
endif ()


if (NOT MSVC)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        BOOMERANG_ADD_COMPILE_FLAGS(-O0)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
        # No special flags
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
        BOOMERANG_ADD_COMPILE_FLAGS(-Os)
    else () # Release
        BOOMERANG_ADD_COMPILE_FLAGS(-O3)
    endif ()

    if (BOOMERANG_ENABLE_COVERAGE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs")
    endif (BOOMERANG_ENABLE_COVERAGE)
endif (NOT MSVC)

if (CMAKE_C_COMPILER MATCHES "[Cc]lang")
    option(BOOMERANG_USE_ASAN "Use Clang address sanitizer (-fsanitize=address)" OFF)
    if (BOOMERANG_USE_ASAN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif ()
endif ()
