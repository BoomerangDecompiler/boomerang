set(target_INCLUDE_DIR
    ..
)
include_directories(${target_INCLUDE_DIR}
    ../../frontend/sparc
    ../../frontend/pentium
)

set(test_LIBRARIES
${PROTOBUF_LIBRARIES}
${GC_LIBS}
${DEBUG_LIB}
boom_base frontend db type boomerang_DSLs codegen util BinaryFile
boom_base frontend db codegen
pthread
)
function(ADD_QTEST NAME)
  add_executable(${NAME} ${NAME}.cpp ${NAME}.h) #${PROTO_SRCS} ${PROTO_HDRS}
  target_link_libraries(${NAME} ${test_LIBRARIES})
  qt5_use_modules(${NAME} Core Test)

  add_test( NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}> )
endfunction()
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
