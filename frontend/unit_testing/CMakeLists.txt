INCLUDE_DIRECTORIES(
    ..
    ../../frontend/sparc
    ../../frontend/pentium
)
set(test_LIBRARIES
${PROTOBUF_LIBRARIES}
${GC_LIBS}
${DEBUG_LIB}
boom_base frontend db type boomerang_DSLs codegen util
boom_base frontend db codegen boomerang_passes
pthread
)

set(TESTS
    FrontendTest
    FrontPentTest
    FrontSparcTest
)
foreach(t ${TESTS})
  ADD_QTEST(${t})
endforeach()

