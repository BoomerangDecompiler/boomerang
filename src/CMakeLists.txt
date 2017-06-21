
# generate config.h
configure_file(include/config.h.cmake include/config.h)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}

	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_BINARY_DIR}" # for generated files like config.h
	"${CMAKE_CURRENT_SOURCE_DIR}/libboomerang"
)

link_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/loader" # for frontend loaders
)

#all submodules
add_subdirectory(libboomerang)
add_subdirectory(c)
add_subdirectory(codegen)
add_subdirectory(type)
add_subdirectory(db)
add_subdirectory(frontend)
add_subdirectory(transform)
add_subdirectory(passes)

add_subdirectory(loader)
add_subdirectory(ui)
