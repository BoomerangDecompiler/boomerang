SET(SRC
        util.cpp
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
ADD_LIBRARY(util STATIC ${SRC})
qt5_use_modules(util Core)
