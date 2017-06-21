# Include directories needed for all test modules
include_directories(
	"${CMAKE_SOURCE_DIR}/src/"
	"${CMAKE_BINARY_DIR}/src/"
	"${CMAKE_SOURCE_DIR}/src/libboomerang/"
)

#add submodlules for testing
add_subdirectory(boom_base)
add_subdirectory(c)
add_subdirectory(db)
add_subdirectory(frontend)
add_subdirectory(loader)
add_subdirectory(type)

