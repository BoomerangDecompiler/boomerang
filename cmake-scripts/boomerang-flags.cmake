#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


# Check for required compiler version
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # Reject Clang < 3.9
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.9)
        message(FATAL_ERROR "Your Clang version is too old.\n\
            Please upgrade Clang or select another compiler.")
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    # Reject GCC < 4.8
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "Your GCC version is too old.\n\
            Please upgrade GCC or select another compiler.")
    endif ()
endif ()


# Force C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(COMMON_COMPILE_FLAGS "")
set(C_COMPILE_FLAGS "")
set(CXX_COMPILE_FLAGS "")
set(LINKER_FLAGS "")

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

# This function adds the flag(s) to the c++ compiler flags
function(AddCompileFlags)
    set(C_COMPILE_FLAGS "")
    set(CXX_COMPILE_FLAGS "")

    foreach (flag ${ARGN})
        # We cannot check for -Wno-foo as this won't throw a warning so we must check for the -Wfoo option directly
        # http://stackoverflow.com/questions/38785168/cc1plus-unrecognized-command-line-option-warning-on-any-other-warning
        string(REGEX REPLACE "^-Wno-" "-W" checkedFlag ${flag})
        set(VarName ${checkedFlag})
        string(REPLACE "+" "X" VarName ${VarName})
        string(REGEX REPLACE "[-=]" "_" VarName ${VarName})

        # Avoid double checks. A compiler will not magically support a flag it did not before
        if (NOT ${VarName}_CHECKED)
            CHECK_CXX_COMPILER_FLAG(${checkedFlag} CXX_FLAG_${VarName}_SUPPORTED)
            CHECK_C_COMPILER_FLAG(${checkedFlag}   C_FLAG_${VarName}_SUPPORTED)
            set(${VarName}_CHECKED YES CACHE INTERNAL "")
        endif()

        if (CXX_FLAG_${VarName}_SUPPORTED)
            set(CXX_COMPILE_FLAGS "${C_COMPILE_FLAGS} ${flag}")
        endif ()
        if (C_FLAG_${VarName}_SUPPORTED)
            set(C_COMPILE_FLAGS "${C_COMPILE_FLAGS} ${flag}")
        endif ()

        unset(VarName)
        unset(checkedFlag)
    endforeach ()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_COMPILE_FLAGS}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${C_COMPILE_FLAGS}" PARENT_SCOPE)
endfunction()


# Add compiler flags if available
if (MSVC)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS
    )

    AddCompileFlags(/W3 /EHsc)

else () # GCC / Clang
    AddCompileFlags(-Wall -Wextra -Werror -Wshadow)
    AddCompileFlags(-Werror=pedantic)
    AddCompileFlags(-Wformat=2)
    AddCompileFlags(-Wmissing-include-dirs)
    AddCompileFlags(-Wstrict-overflow=2)
    AddCompileFlags(-Wnull-dereference)
    AddCompileFlags(-Wduplicated-cond)
    AddCompileFlags(-Wduplicated-branches)
    AddCompileFlags(-Walloc-zero)
    AddCompileFlags(-Walloca)
    AddCompileFlags(-rdynamic -fPIC)
    AddCompileFlags(-Wno-unknown-pragmas) # pragma region is not supported by GCC
    AddCompileFlags(-Wsuggest-override)
    AddCompileFlags(-fno-strict-aliasing) # Will break *reinterpret-cast<float*>(&int) otherwise
    AddCompileFlags(-Wundef)
    AddCompileFlags(-Wno-gnu-zero-variadic-macro-arguments) # Will break QSKIP() macro on clang otherwise

    if (Qt5Core_VERSION VERSION_GREATER 5.6.1)
        # See https://bugreports.qt.io/browse/QTBUG-45291
        AddCompileFlags(-Wzero-as-null-pointer-constant)
    endif ()

    # Do not treat specific warnings as errors
    AddCompileFlags(-Wno-error=strict-overflow)
    AddCompileFlags(-Wno-error=alloca)

    # Other warnings
#    AddCompileFlags(-Wcast-qual)
#    AddCompileFlags(-Wconversion)
#    AddCompileFlags(-Wswitch-enum)
endif ()


if (NOT MSVC)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        AddCompileFlags(-g -O0)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
        AddCompileFlags(-g -O2)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
        AddCompileFlags(-Os)
    else () # Release
        AddCompileFlags(-O3)
    endif ()
endif (NOT MSVC)
