
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

include(boomerang-utils)
BOOMERANG_COPY_IMPORTED_DLL(boomerang Qt5::Core)
BOOMERANG_COPY_IMPORTED_DLL(boomerang Qt5::Xml)

# install library and headers
install(TARGETS boomerang
    ARCHIVE DESTINATION lib/
    LIBRARY DESTINATION lib/
    RUNTIME DESTINATION bin/
)

install(DIRECTORY . DESTINATION include/boomerang
    FILES_MATCHING PATTERN "*.h"
)

