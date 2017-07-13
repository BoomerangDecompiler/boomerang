
set(SRC
    pentium/pentiumdecoder.cpp
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

add_library(boomerang-frontend STATIC ${SRC})
target_link_libraries(boomerang-frontend boomerang)

set(SSL_FILES
	machine/st20/st20.ssl
	machine/mips/mips.ssl
	machine/hppa/hppa.ssl
	machine/pentium/pentium.ssl
	machine/ppc/ppc.ssl
	machine/sparc/sparc.ssl
)

foreach (SSLFile ${ssl_file})
	BOOMERANG_COPY(
		TARGET boomerang-frontend PRE_BUILD
		FILES ${ssl_file}
		DESTINATION "${BOOMERANG_OUTPUT_DIR}/frontend/${ssl_file}"
	)
endforeach ()

# installation
install(TARGETS boomerang-frontend
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)
