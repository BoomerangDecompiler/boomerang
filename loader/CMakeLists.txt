ADD_LIBRARY(BinaryFile       SHARED  SymTab.cpp  BinaryFile.cpp)
ADD_LIBRARY(ElfBinaryFile    SHARED  SymTab.cpp  ElfBinaryFile.cpp)
ADD_LIBRARY(Win32BinaryFile  SHARED  SymTab.cpp  Win32BinaryFile.cpp microX86dis.c)
ADD_LIBRARY(ExeBinaryFile    SHARED  SymTab.cpp  ExeBinaryFile.cpp)
ADD_LIBRARY(HpSomBinaryFile  SHARED  SymTab.cpp  HpSomBinaryFile.cpp)
ADD_LIBRARY(PalmBinaryFile   SHARED  SymTab.cpp  PalmBinaryFile.cpp)
ADD_LIBRARY(DOS4GWBinaryFile SHARED  SymTab.cpp  DOS4GWBinaryFile.cpp)

ADDCFLAG(MachOBinaryFile.cpp "-I${CMAKE_CURRENT_SOURCE_DIR}")
ADD_LIBRARY(MachOBinaryFile  SHARED  SymTab.cpp  MachOBinaryFile.cpp)

ADD_EXECUTABLE(bffDump bffDump.cpp BinaryFileFactory.cpp)
TARGET_LINK_LIBRARIES(bffDump ${DL_LIBS})
TARGET_LINK_LIBRARIES(bffDump BinaryFile)

ADD_EXECUTABLE(testLoader testLoader.cpp LoaderTest.cpp BinaryFileFactory.cpp microX86dis.c SymTab.cpp)
TARGET_LINK_LIBRARIES(testLoader ${CPPUNIT_LIBS})
TARGET_LINK_LIBRARIES(testLoader ${DL_LIBS})
TARGET_LINK_LIBRARIES(testLoader BinaryFile)