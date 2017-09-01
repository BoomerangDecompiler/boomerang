
set(boomerang-sources "")

add_subdirectory(c)
add_subdirectory(codegen)
add_subdirectory(core)
add_subdirectory(db)
add_subdirectory(frontend)
add_subdirectory(passes)
add_subdirectory(transform)
add_subdirectory(type)
add_subdirectory(util)


add_library(boomerang ${boomerang-sources})
target_link_libraries(boomerang ${CMAKE_DL_LIBS} Qt5::Core ${DEBUG_LIB})

set_target_properties(boomerang PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

# install library and headers
install(TARGETS boomerang
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY . DESTINATION include/boomerang
        FILES_MATCHING PATTERN "*.h")

