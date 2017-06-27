
set(SRC
    pentium/pentiumdecoder.cpp #-fno-exceptions
    pentium/pentiumfrontend.cpp
    ppc/ppcdecoder.cpp
    ppc/ppcfrontend.cpp
    sparc/sparcdecoder.cpp
    sparc/sparcdecoder.h
    sparc/sparcfrontend.cpp
    sparc/sparcfrontend.h
#	mc68k/m68kfrontend.cpp
#	mc68k/m68kdecoder.cpp
    mips/mipsdecoder.cpp
    mips/mipsdecoder.h
    mips/mipsfrontend.cpp
    mips/mipsfrontend.h
    st20/st20decoder.cpp
    st20/st20decoder.h
    st20/st20frontend.cpp
    st20/st20frontend.h
)

set(SSL_FILES
	machine/st20/st20.ssl
	machine/mips/mips.ssl
	machine/hppa/hppa.ssl
	machine/pentium/pentium.ssl
	machine/ppc/ppc.ssl
	machine/sparc/sparc.ssl
)

foreach(ssl_file ${SSL_FILES})
	add_custom_command(
		OUTPUT "${BOOMERANG_OUTPUT_DIR}/frontend/${ssl_file}"
		COMMAND cmake -E copy
		"${CMAKE_CURRENT_SOURCE_DIR}/${ssl_file}"
		"${BOOMERANG_OUTPUT_DIR}/frontend/${ssl_file}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${ssl_file}"
	)
	list(APPEND ssl_files_dest "${BOOMERANG_OUTPUT_DIR}/frontend/${ssl_file}" )
endforeach()

add_custom_target(CopySSLs ALL DEPENDS ${ssl_files_dest})

add_library(boomerang-frontend STATIC ${SRC})
target_link_libraries(boomerang-frontend boomerang)

add_dependencies(boomerang-frontend CopySSLs)

# installation
install(TARGETS boomerang-frontend
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)
