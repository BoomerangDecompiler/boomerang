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
)

if (NOT WIN32)
list(APPEND test_LIBRARIES pthread)
endif()

IF(MSVC)
list(APPEND test_LIBRARIES Dbghelp.lib)
ENDIF()

set(TESTS
    FrontendTest
    FrontPentTest
    FrontSparcTest
)
foreach(t ${TESTS})
  ADD_QTEST(${t})
endforeach()

