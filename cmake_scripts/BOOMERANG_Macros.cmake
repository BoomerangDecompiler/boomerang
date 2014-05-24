#this macro expects all loader names to be the same as their cpp files
# collected names will be available in local scoped variable named
# _BOOMERANG_...

MACRO(BOOMERANG_ADD_LOADER loader_name)
	IF(NOT ${loader_name}_LOADER_VISITED)
		# add the loader as a dll
		ADD_LIBRARY(${loader_name} MODULE 
					${loader_name}.cpp 
					${loader_name}.h
					${PROJECT_SOURCE_DIR}/loader/SymTab.cpp 
					${PROJECT_SOURCE_DIR}/loader/SymTab.h
					${ARGN})
		# all loaders depend on BinaryFile
		TARGET_LINK_LIBRARIES(${loader_name} BinaryFile)
		ADD_DEPENDENCIES(${loader_name} BinaryFile)
		# add this loader to project's global loader list
		LIST(APPEND _BOOMERANG_LOADERS ${loader_name})		
		SET(${loader_name}_LOADER_VISITED true)
	ENDIF(NOT ${loader_name}_LOADER_VISITED)
ENDMACRO(BOOMERANG_ADD_LOADER)

# this macro will try to add (frontend_name+decoder) and 
# (frontend_name+frontend) as sources
MACRO(BOOMERANG_ADD_FRONTEND frontend_name)
	IF(NOT ${frontend_name}_FRONTEND_VISITED)
		# add the loader as a static
		ADD_LIBRARY(Frontend_${frontend_name} STATIC
		${frontend_name}decoder.cpp
		${frontend_name}frontend.cpp
		${frontend_name}decoder.h
		${frontend_name}frontend.h
		 ${ARGN}
		 )
		# add this generator to project's global loader list
		LIST(APPEND _BOOMERANG_FRONTENDS Frontend_${frontend_name})		
		SET(${frontend_name}_FRONTEND_VISITED true)
	ENDIF(NOT ${frontend_name}_FRONTEND_VISITED)
ENDMACRO(BOOMERANG_ADD_FRONTEND)

MACRO(BOOMERANG_ADD_CODEGEN gen_name)
	IF(NOT ${gen_name}_CODEGEN_VISITED)
		# add the loader as a static
		ADD_LIBRARY(Codegen_${gen_name} STATIC ${ARGN})
		# add this generator to project's global loader list
		LIST(APPEND _BOOMERANG_CODE_GENERATORS Codegen_${gen_name})		
		SET(${gen_name}_CODEGEN_VISITED true)
	ENDIF(NOT ${gen_name}_CODEGEN_VISITED)
ENDMACRO(BOOMERANG_ADD_CODEGEN)

MACRO(ADD_UNIT_TEST name)
	IF(NOT ${name}_TEST_VISITED)
		# add the loader as a dll
		ADD_EXECUTABLE(${name} ${ARGN})
		MESSAGE(WARNING "Adding test " ${name} " " ${ARGN})
		TARGET_LINK_LIBRARIES(${name} ${UNIT_TEST_LIBS})
		ADD_TEST(NAME ${name} COMMAND ${name})
		SET(${name}_TEST_VISITED true)
	ENDIF()
ENDMACRO()

