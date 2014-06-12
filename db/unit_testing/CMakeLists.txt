set(target_INCLUDE_DIR
    ..
)
include_directories(${target_INCLUDE_DIR}
    ../../frontend/sparc
    ../../frontend/pentium
)
include(BOOMERANG_Macros)
set(test_LIBRARIES
${PROTOBUF_LIBRARIES}
${GC_LIBS}
${DEBUG_LIB}
boom_base frontend db type boomerang_DSLs codegen util BinaryFile
boom_base frontend db codegen boomerang_passes
pthread
)
set(TESTS
    RtlTest
    CfgTest
    DfaTest
    ParserTest
)
foreach(t ${TESTS})
  ADD_QTEST(${t})
endforeach()

#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS codeblock.proto)
