SET(SRC
    constraint.cpp
    dfa.cpp
    type.cpp
)
ADD_LIBRARY(type STATIC ${SRC})
qt5_use_modules(type Core)

IF(BUILD_TESTING)
ADD_SUBDIRECTORY(unit_testing)
ENDIF()

