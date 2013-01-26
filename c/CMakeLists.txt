SET(ansi_c_SRC 
    ansi-c-parser.cpp
    ansi-c-scanner.cpp
)
ADD_LIBRARY(ansi-c-parser STATIC ${ansi_c_SRC})