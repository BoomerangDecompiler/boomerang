
list(APPEND boomerang-passes-sources
    passes/Pass
    passes/RangeAnalysis
)

set(boomerang-sources "${boomerang-sources};${boomerang-passes-sources}" PARENT_SCOPE)
