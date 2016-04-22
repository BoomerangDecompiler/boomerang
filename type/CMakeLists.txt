SET(SRC
    constraint.cpp
    constraint.h
    dfa.cpp
    type.cpp
    TypeRecovery.cpp
    TypeRecovery.h
)
ADD_LIBRARY(type STATIC ${SRC})
qt5_use_modules(type Core)

IF(BUILD_TESTING)
ADD_SUBDIRECTORY(unit_testing)
ENDIF()

