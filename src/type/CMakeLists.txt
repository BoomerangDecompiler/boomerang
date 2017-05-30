SET(SRC
    constraint.cpp
    constraint.h
    dfa.cpp
    dfa.h
    type.cpp
    TypeRecovery.cpp
    TypeRecovery.h
)

add_library(type STATIC ${SRC})
qt5_use_modules(type Core)

if (BUILD_TESTING)
	add_subdirectory(unit_testing)
endif ()

