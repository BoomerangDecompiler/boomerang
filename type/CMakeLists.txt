SET(SRC
    constraint.cpp
    dfa.cpp
    type.cpp
)
ADD_LIBRARY(type STATIC ${SRC})
qt5_use_modules(type Core)
