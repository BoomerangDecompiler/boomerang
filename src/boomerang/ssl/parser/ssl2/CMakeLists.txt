
add_custom_target(SSL2ParserGen
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/SSL2Scanner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SSL2Parser.cpp
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/SSL2Scanner.cpp
    COMMAND flex ARGS -o ${CMAKE_CURRENT_SOURCE_DIR}/SSL2Scanner.cpp ${CMAKE_CURRENT_SOURCE_DIR}/SSL2Scanner.l
    MAIN_DEPENDENCY SSL2Scanner.l
    VERBATIM
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/SSL2Parser.cpp
    COMMAND bison ARGS -o${CMAKE_CURRENT_SOURCE_DIR}/SSL2Parser.cpp ${CMAKE_CURRENT_SOURCE_DIR}/SSL2Parser.y
    MAIN_DEPENDENCY SSL2Parser.y
    BYPRODUCTS location.hh position.hh stack.hh SSL2Parser.hpp
    VERBATIM
)

add_library(boomerang-ssl2-parser STATIC
    location.hh
    position.hh
    SSL2ParserDriver.cpp
    SSL2ParserDriver.h
    SSL2Parser.cpp
    SSL2Parser.hpp
    SSL2Scanner.cpp
    stack.hh
)

target_link_libraries(boomerang-ssl2-parser Qt5::Core)

add_dependencies(boomerang-ssl2-parser SSL2ParserGen)
