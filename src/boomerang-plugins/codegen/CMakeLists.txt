
include(boomerang-utils)

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
)

BOOMERANG_ADD_CODEGEN(
    NAME "C"
    SOURCES
        c/CCodeGenerator.cpp
        c/CCodeGenerator.h
        c/CodeWriter.cpp
        c/CodeWriter.h
        c/ControlFlowAnalyzer.cpp
        c/ControlFlowAnalyzer.h
)
