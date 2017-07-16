
set(boomerang_SRC
    c/ansi-c-parser.cpp
    c/ansi-c-scanner.cpp
    c/ansi-c-parser.h
    c/ansi-c-scanner.h
    
    codegen/CCodeGenerator.cpp
    codegen/CCodeGenerator.h
    codegen/ICodeGenerator.h
    codegen/SyntaxNode.h
    codegen/SyntaxNode.cpp
    
    core/Boomerang.h
    core/Boomerang.cpp
    core/BinaryFileFactory.cpp
    core/Plugin.h
    core/Plugin.cpp
    
    db/SymTab
    db/SectionInfo
    db/IBinarySection.h
    db/IBinaryImage.h
    db/BinaryImage
    db/IBinarySymbols.h
    db/basicblock.h
    db/basicblock.cpp
    db/cfg.h
    db/cfg.cpp
    db/dataflow.h
    db/dataflow.cpp
    db/exp.h
    db/exp.cpp
    db/exphelp.h
    db/insnameelem.cpp
    db/managed.cpp
    db/proc.h
    db/proc.cpp
    db/prog.h
    db/prog.cpp
    db/module.h
    db/module.cpp
    db/project.h
    db/project.cpp
    db/IProject.h
    db/register.h
    db/register.cpp
    db/rtl.h
    db/rtl.cpp
    db/signature.h
    db/signature.cpp
    
    db/ssl/sslinst.cpp
    db/ssl/sslparser.cpp
    db/ssl/sslparser.h
    db/ssl/sslparser_support.cpp
    db/ssl/sslscanner.cpp
    db/ssl/sslscanner.h
    
    # statement handling
    db/statements/statement.h
    db/statements/statement.cpp
    db/statements/assign.h
    db/statements/assign.cpp
    db/statements/assignment.h
    db/statements/assignment.cpp
    db/statements/boolassign.h
    db/statements/boolassign.cpp
    db/statements/branchstatement.h
    db/statements/branchstatement.cpp
    db/statements/callstatement.h
    db/statements/callstatement.cpp
    db/statements/casestatement.h
    db/statements/casestatement.cpp
    db/statements/gotostatement.h
    db/statements/gotostatement.cpp
    db/statements/implicitassign.h
    db/statements/implicitassign.cpp
    db/statements/imprefstatement.h
    db/statements/imprefstatement.cpp
    db/statements/junctionstatement.h
    db/statements/junctionstatement.cpp
    db/statements/phiassign.h
    db/statements/phiassign.cpp
    db/statements/returnstatement.h
    db/statements/returnstatement.cpp
    db/statements/typingstatement.h
    db/statements/typingstatement.cpp
    db/statements/statementhelper.h
    db/statements/statementhelper.cpp    
    
    db/table.cpp
    db/visitor.h
    db/visitor.cpp

    frontend/decoder.h
    frontend/frontend.h
    frontend/frontend.cpp
    frontend/MachineInstruction.cpp
    frontend/MachineInstruction.h
    frontend/njmcDecoder.cpp
    frontend/njmcDecoder.h
    frontend/TargetQueue.cpp
    frontend/TargetQueue.h
    
    passes/Pass
    passes/RangeAnalysis
    
    transform/transformer.cpp
    transform/rdi.cpp
    transform/generic.cpp
    transform/transformation-parser.cpp
    transform/transformation-scanner.cpp
    transform/rdi.h
    transform/generic.h
    transform/transformation-parser.h
    transform/transformation-scanner.h
    
    type/constraint.cpp
    type/constraint.h
    type/dfa.cpp
    type/dfa.h
    type/type.h
    type/type.cpp
    type/TypeRecovery.cpp
    type/TypeRecovery.h
    
    util/Address.h
    util/Address.cpp
    util/Log.h
    util/Log.cpp
    util/Util.h
    util/Util.cpp
)

add_library(boomerang ${boomerang_SRC})
target_link_libraries(boomerang boomerang-frontend ${CMAKE_DL_LIBS} Qt5::Core ${DEBUG_LIB})

set_target_properties(boomerang PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

# install library and headers
install(TARGETS boomerang
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY . DESTINATION include/boomerang
        FILES_MATCHING PATTERN "*.h")

