#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-util-sources
    util/log/Log
    util/log/ConsoleLogSink
    util/log/FileLogSink
    util/log/SeparateLogger

    util/Address
    util/ArgSourceProvider
    util/ByteUtil
    util/CallGraphDotWriter
    util/CFGDotWriter
    util/ConnectionGraph
    util/DFGWriter
    util/ExpPrinter
    util/ExpDotWriter
    util/ExpSet
    util/LocationSet
    util/MapIterators
    util/OStream
    util/ProgSymbolWriter
    util/StatementList
    util/StatementSet
    util/UseGraphWriter
    util/Util
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-util-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-util-sources}" PARENT_SCOPE)
