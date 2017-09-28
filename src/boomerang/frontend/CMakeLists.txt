#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-frontend-sources
    frontend/Decoder.h
    frontend/Frontend.cpp
    frontend/Frontend.h
#    frontend/machine/hppa/decoder.cpp
#    frontend/machine/hppa/decoder_low.cpp
#    frontend/machine/hppa/disassembler.cpp
#    frontend/machine/hppa/enc.h
#    frontend/machine/hppa/fronthppa.cpp
#    frontend/machine/hppa/hppa-names.cpp
#    frontend/machine/hppa/hppa-names.h
#    frontend/machine/hppa/hppa.pat.cpp
#    frontend/machine/hppa/hppa.pat.h
#    frontend/machine/hppa/simple.cpp
    frontend/MachineInstruction.cpp
    frontend/MachineInstruction.h
#    frontend/machine/mc68k/decoder.cpp
#    frontend/machine/mc68k/decoder_low.cpp
#    frontend/machine/mc68k/front68k.cpp
#    frontend/machine/mc68k/mc68k.pat.cpp
#    frontend/machine/mc68k/mc68k.pat.h
#    frontend/machine/mips/disassembler.cpp
#    frontend/machine/sparc/disassembler.cpp
#    frontend/machine/sparc/sparc.h
#    frontend/machine/sparc/sparc-names.h
#    frontend/machine/sparc/sparc.pat.cpp
#    frontend/machine/sparc/sparc.pat.h
    frontend/mips/mipsdecoder.cpp
    frontend/mips/mipsdecoder.h
    frontend/mips/mipsfrontend.cpp
    frontend/mips/mipsfrontend.h
    frontend/NJMCDecoder.cpp
    frontend/NJMCDecoder.h
    frontend/pentium/pentiumdecoder.cpp
    frontend/pentium/pentiumdecoder.h
    frontend/pentium/pentiumfrontend.cpp
    frontend/pentium/pentiumfrontend.h
    frontend/ppc/ppcdecoder.cpp
    frontend/ppc/ppcdecoder.h
    frontend/ppc/ppcfrontend.cpp
    frontend/ppc/ppcfrontend.h
    frontend/SigEnum.h
    frontend/sparc/sparcdecoder.cpp
    frontend/sparc/sparcdecoder.h
    frontend/sparc/sparcfrontend.cpp
    frontend/sparc/sparcfrontend.h
    frontend/st20/st20decoder.cpp
    frontend/st20/st20decoder.h
    frontend/st20/st20frontend.cpp
    frontend/st20/st20frontend.h
    frontend/TargetQueue.cpp
    frontend/TargetQueue.h
)

set(boomerang-sources "${boomerang-sources};${boomerang-frontend-sources}" PARENT_SCOPE)


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
        TARGET boomerang POST_BUILD
        FILES ${ssl_file}
        DESTINATION "${BOOMERANG_OUTPUT_DIR}/frontend/${ssl_file}"
    )
endforeach ()


# installation
install(FILES ${SSL_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/boomerang/frontend/")


