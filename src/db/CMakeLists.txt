include_directories(../c/) # used by prog.cpp

set(INCLUDES
	../include/hllcode.h
	../include/memo.h
	../include/rtl.h
	../include/statement.h
	../include/type.h
	../include/visitor.h
	../include/operator.h
	../include/sigenum.h
	../include/TargetQueue.h
	../include/types.h
	../include/xmlprogparser.h
	../include/decoder.h
	../include/frontend.h
	../include/managed.h
	../include/osfcn.h
	../include/signature.h
	../include/transformer.h
	../include/util.h
	../include/IBoomerang.h
)


set(SRC
	SymTab
	SectionInfo
	IBinaryImage.h
	BinaryImage
	IBinarySymbols.h
	basicblock.h
    basicblock.cpp
    cfg.h
    cfg.cpp
    dataflow.h
    dataflow.cpp
    exp.h
    exp.cpp
    exphelp.h
    insnameelem.cpp
    managed.cpp
    proc.h
    proc.cpp
    prog.h
    prog.cpp
    module.h
    module.cpp
    project.h
    project.cpp
    IProject.h
    register.h
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

