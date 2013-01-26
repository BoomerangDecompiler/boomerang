INCLUDE_DIRECTORIES(../c/) # used by prog.cpp
SET(SRC
        basicblock.cpp
        ../include/basicblock.h
        cfg.cpp
        dataflow.cpp
        exp.cpp
        insnameelem.cpp
        managed.cpp
        proc.cpp
        prog.cpp #-Icodegen -Ic
        register.cpp
        rtl.cpp
        signature.cpp
        sslinst.cpp
        sslparser.cpp
        sslscanner.cpp
        statement.cpp
        table.cpp
        visitor.cpp
)
IF(EXPAT_FOUND)
    SET(SRC ${SRC} xmlprogparser.cpp)
ENDIF()

ADD_LIBRARY(db STATIC ${SRC})
