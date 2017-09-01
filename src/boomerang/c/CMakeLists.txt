
list(APPEND boomerang-c-sources
    c/ansi-c-parser.cpp
    c/ansi-c-scanner.cpp
    c/ansi-c-parser.h
    c/ansi-c-scanner.h
)

set(boomerang-sources "${boomerang-sources};${boomerang-c-sources}" PARENT_SCOPE)
