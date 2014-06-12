SET(boomerang_SRC
    commandlinedriver.cpp
    commandlinedriver.h
    driver.cpp
)

ADD_EXECUTABLE(boomerang ${boomerang_SRC})
TARGET_LINK_LIBRARIES(boomerang
${GC_LIBS}
${DEBUG_LIB}
boom_base frontend db type boomerang_DSLs codegen util BinaryFile boom_base
pthread boomerang_passes
)
qt5_use_modules(boomerang Core Xml)
