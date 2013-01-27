SET(boomerang_c_codegen_sources
        chllcode.cpp
        syntax.cpp
chllcode.h
)
ADD_LIBRARY(codegen STATIC ${boomerang_c_codegen_sources})
