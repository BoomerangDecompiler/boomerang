# This macro expects all loader names to be the same as their cpp files.
# Collected names will be available in local scoped variable named
# _BOOMERANG_...
macro(BOOMERANG_ADD_LOADER loader_name)
	if (NOT ${loader_name}_LOADER_VISITED)
		# add the loader as a dll
		add_library(${loader_name} MODULE
			${loader_name}.cpp
			${loader_name}.h
			${PROJECT_SOURCE_DIR}/loader/SymTab.cpp
			${PROJECT_SOURCE_DIR}/loader/SymTab.h
			${ARGN}
		)
	
		# all loaders depend on BinaryFile
		target_link_libraries(${loader_name} BinaryFile)
		add_dependiencies(${loader_name} BinaryFile)
		qt5_use_modules(${loader_name} Core)
	
		# add this loader to project's global loader list
		list(APPEND _BOOMERANG_LOADERS ${loader_name})
		
		set(${loader_name}_LOADER_VISITED true)
	endif (NOT ${loader_name}_LOADER_VISITED)
endmacro(BOOMERANG_ADD_LOADER)


# This macro will try to add (frontend_name+decoder) and
# (frontend_name+frontend) as sources
macro(BOOMERANG_ADD_FRONTEND frontend_name)
	if (NOT ${frontend_name}_FRONTEND_VISITED)
		# add the loader as a static
		add_library(Frontend_${frontend_name} STATIC
			${frontend_name}decoder.cpp
			${frontend_name}frontend.cpp
			${frontend_name}decoder.h
			${frontend_name}frontend.h
			${ARGN}
		)
		
		qt5_use_modules(${frontend_name} Core)
		
		# add this generator to project's global loader list
		list(APPEND _BOOMERANG_FRONTENDS Frontend_${frontend_name})
		
		set(${frontend_name}_FRONTEND_VISITED true)
	endif (NOT ${frontend_name}_FRONTEND_VISITED)
endmacro(BOOMERANG_ADD_FRONTEND)


macro(BOOMERANG_ADD_CODEGEN gen_name)
	if (NOT ${gen_name}_CODEGEN_VISITED)
		# add the loader as a static
		add_library(Codegen_${gen_name} STATIC ${ARGN})
		qt5_use_modules(Codegen_${gen_name} Core)
		
		# add this generator to project's global loader list
		list(APPEND _BOOMERANG_CODE_GENERATORS Codegen_${gen_name})
		set(${gen_name}_CODEGEN_VISITED true)
	endif(NOT ${gen_name}_CODEGEN_VISITED)
endmacro(BOOMERANG_ADD_CODEGEN)


macro(ADD_UNIT_TEST name)
	if (NOT ${name}_TEST_VISITED)
		# add the loader as a dll
		add_executable(${name} ${ARGN})
		qt5_use_modules(${name} Core)
		
		message(WARNING "Adding test " ${name} " " ${ARGN})
		target_link_libraries(${name} ${UNIT_TEST_LIBS})
		
		add_test(NAME ${name} COMMAND ${name})
		set_property(TEST ${name} APPEND PROPERTY ENVIRONMENT BOOMERANG_TEST_BASE=${PROJECT_SOURCE_DIR})
		set(${name}_TEST_VISITED true)
	endif ()
endmacro(ADD_UNIT_TEST)


function(ADD_QTEST NAME)
	add_executable(${NAME} ${NAME}.cpp ${NAME}.h)
	target_link_libraries(${NAME} ${test_LIBRARIES})
	qt5_use_modules(${NAME} Core Test)

	add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
	set_property(TEST ${NAME} APPEND PROPERTY ENVIRONMENT BOOMERANG_TEST_BASE=${PROJECT_SOURCE_DIR})
endfunction()

