INCLUDE_DIRECTORIES(../db/) # for operstrings
SET(SRC
        chllcode.cpp
        syntax.cpp
)
ADD_LIBRARY(codegen STATIC ${SRC})
