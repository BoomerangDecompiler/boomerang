SET(target_CPP
    RtlTest.cpp
    CfgTest.cpp
    DfaTest.cpp
    ParserTest.cpp
)
SET(target_INCLUDE
    RtlTest.h
)

SET(target_SOURCES
    ${target_CPP}
    ${target_INCLUDE}
)
SET(target_INCLUDE_DIR
    ..
)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR}
    ../../frontend/sparc
    ../../frontend/pentium
)

#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS codeblock.proto)

ADD_EXECUTABLE(db_tests ${target_SOURCES} ) #${PROTO_SRCS} ${PROTO_HDRS}
ADD_DEPENDENCIES(db_tests GMOCK_BUILD)
TARGET_LINK_LIBRARIES(db_tests ${GTest_LIBRARIES} ${PROTOBUF_LIBRARIES}
${DL_LIBS}
${GC_LIBS}
${DEBUG_LIB}
"${EXPAT_LIBRARIES}"
boom_base frontend db type boomerang_DSLs codegen util BinaryFile
boom_base frontend db codegen
pthread
)
qt5_use_modules(db_tests Core)
