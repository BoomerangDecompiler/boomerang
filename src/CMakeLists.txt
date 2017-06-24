

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} # for generated source files
)


add_subdirectory(boomerang)
add_subdirectory(boomerang-cli)
add_subdirectory(boomerang-gui)
add_subdirectory(boomerang-frontend)

# plugins
add_subdirectory(boomerang-loaders)

