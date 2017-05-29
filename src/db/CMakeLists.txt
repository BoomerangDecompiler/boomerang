include_directories(${CMAKE_SOURCE_DIR}/src/c/) # used by prog.cpp

set(INCLUDES
	${CMAKE_SOURCE_DIR}/include/basicblock.h
	${CMAKE_SOURCE_DIR}/include/boomerang.h
	${CMAKE_SOURCE_DIR}/include/exp.h
	${CMAKE_SOURCE_DIR}/include/hllcode.h
	${CMAKE_SOURCE_DIR}/include/memo.h
	${CMAKE_SOURCE_DIR}/include/proc.h
	${CMAKE_SOURCE_DIR}/include/rtl.h
	${CMAKE_SOURCE_DIR}/include/statement.h
	${CMAKE_SOURCE_DIR}/include/type.h
	${CMAKE_SOURCE_DIR}/include/visitor.h
	${CMAKE_SOURCE_DIR}/include/BinaryFile.h
	${CMAKE_SOURCE_DIR}/include/cfg.h
	${CMAKE_SOURCE_DIR}/include/dataflow.h
	${CMAKE_SOURCE_DIR}/include/exphelp.h
	${CMAKE_SOURCE_DIR}/include/log.h
	${CMAKE_SOURCE_DIR}/include/operator.h
	${CMAKE_SOURCE_DIR}/include/prog.h
	${CMAKE_SOURCE_DIR}/include/sigenum.h
	${CMAKE_SOURCE_DIR}/include/TargetQueue.h
	${CMAKE_SOURCE_DIR}/include/types.h
	${CMAKE_SOURCE_DIR}/include/xmlprogparser.h
	${CMAKE_SOURCE_DIR}/include/BinaryFileStub.h
	${CMAKE_SOURCE_DIR}/include/module.h
	${CMAKE_SOURCE_DIR}/include/decoder.h
	${CMAKE_SOURCE_DIR}/include/frontend.h
	${CMAKE_SOURCE_DIR}/include/managed.h
	${CMAKE_SOURCE_DIR}/include/osfcn.h
	${CMAKE_SOURCE_DIR}/include/project.h
	${CMAKE_SOURCE_DIR}/include/register.h
	${CMAKE_SOURCE_DIR}/include/signature.h
	${CMAKE_SOURCE_DIR}/include/transformer.h
	${CMAKE_SOURCE_DIR}/include/util.h
	${CMAKE_SOURCE_DIR}/include/IBinaryImage.h
	${CMAKE_SOURCE_DIR}/include/IBinarySymbols.h
	${CMAKE_SOURCE_DIR}/include/IBoomerang.h
	${CMAKE_SOURCE_DIR}/include/IProject.h
)

set(SRC
	SymTab
	SectionInfo
	BinaryImage
    basicblock.cpp
    cfg.cpp
    dataflow.cpp
    exp.cpp
    insnameelem.cpp
    managed.cpp
    proc.cpp
    prog.cpp
    module.cpp
    project.cpp
    register.cpp
    rtl.cpp
    signature.cpp
    sslinst.cpp
    sslparser.cpp
    sslparser_support.cpp
    sslscanner.cpp
    statement.cpp
    table.cpp
    visitor.cpp
#	xmlprogparser.cpp
)

if (BUILD_TESTING)
	add_subdirectory(unit_testing)
endif ()

add_library(db STATIC ${SRC} ${INCLUDES})

qt5_use_modules(db Core Xml)

