
set(boomerang-cli-sources
    Console.h
    Console.cpp
    CommandlineDriver.h
    CommandlineDriver.cpp
    main.cpp
)

add_executable(boomerang-cli
    ${boomerang-cli-sources}
)

target_link_libraries(boomerang-cli
    boomerang
    ${CMAKE_DL_LIBS}
    Qt5::Core
)

install(TARGETS boomerang-cli
    RUNTIME DESTINATION bin/
)

