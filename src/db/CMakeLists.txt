include_directories(../c/) # used by prog.cpp

set(INCLUDES
	../include/hllcode.h
	../include/memo.h
	../include/type.h
	../include/operator.h
	../include/sigenum.h
	../include/TargetQueue.h
	../include/types.h
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
    
    # statement handling
    statements/statement.h
    statements/statement.cpp
	statements/assign.h
	statements/assign.cpp
	statements/assignment.h
	statements/assignment.cpp
	statements/boolassign.h
	statements/boolassign.cpp
	statements/branchstatement.h
	statements/branchstatement.cpp
	statements/callstatement.h
	statements/callstatement.cpp
	statements/casestatement.h
	statements/casestatement.cpp
	statements/gotostatement.h
	statements/gotostatement.cpp
	statements/implicitassign.h
	statements/implicitassign.cpp
	statements/imprefstatement.h
	statements/imprefstatement.cpp
	statements/junctionstatement.h
	statements/junctionstatement.cpp
	statements/phiassign.h
	statements/phiassign.cpp
	statements/returnstatement.h
	statements/returnstatement.cpp
	statements/typingstatement.h
	statements/typingstatement.cpp
	statements/statementhelper.h
	statements/statementhelper.cpp
	
    table.cpp
    visitor.h
    visitor.cpp
	#xmlprogparser.cpp
	#xmlprogparser.h
	
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

