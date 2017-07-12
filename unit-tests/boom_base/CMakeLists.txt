
SET(target_CPP
    basic_tests.cpp
    testloader.cpp
)

SET(target_INCLUDE
    testloader.h
    basic_tests.h
)

SET(target_SOURCES
    ${target_CPP}
    ${target_INCLUDE}
)

add_executable(g_tests ${target_SOURCES})

target_link_libraries(g_tests
	${GTest_LIBRARIES}
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT})

qt5_use_modules(g_tests Core Test)

add_test(NAME g_tests COMMAND $<TARGET_FILE:g_tests>)

