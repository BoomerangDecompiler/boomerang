
list(APPEND boomerang-type-sources
    type/Constraint.cpp
    type/Constraint.h
    type/DFATypeRecovery.cpp
    type/DFATypeRecovery.h
    type/Type.h
    type/Type.cpp
    type/TypeRecovery.cpp
    type/TypeRecovery.h
)

set(boomerang-sources "${boomerang-sources};${boomerang-type-sources}" PARENT_SCOPE)
