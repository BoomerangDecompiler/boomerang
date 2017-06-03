include_directories(../c/) # used by prog.cpp

set(INCLUDES
	../include/hllcode.h
	../include/memo.h
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
	../include/transformer.h
	../include/util.h
	../include/IBoomerang.h
)


set(SRC
	SymTab
	SectionInfo
	IBinarySection.h
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
    rtl.h
    rtl.cpp
    signature.h
    signature.cpp
    
    statement.h
    statement.cpp
    table.cpp
    visitor.cpp
#	xmlprogparser.cpp

    #ssl handling
    ssl/sslinst.cpp
    ssl/sslparser.cpp
    ssl/sslparser.h
    ssl/sslparser_support.cpp
    ssl/sslscanner.cpp
    ssl/sslscanner.h
)

add_library(db STATIC ${SRC} ${INCLUDES})

qt5_use_modules(db Core Xml)

