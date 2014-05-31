/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002-2006, Mike Van Emmerik and Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file        driver.cpp
  * \brief    Important initialisation that has to happen at the start of main()
  *                Also contains main(), so it can be the only file different between boomerang and bigtest
  ***************************************************************************/

#include <QtCore>
#include <cstdio>
#include "commandlinedriver.h"
#include "boomerang.h"
#include "config.h"
#ifdef HAVE_LIBGC
//#define GC_DEBUG 1        // Uncomment to debug the garbage collector
#include "gc.h"
#else
#define NO_NEW_OR_DELETE_OPERATORS
#define NO_GARBAGE_COLLECTOR
#endif
void init_dfa();        // Prototypes for
void init_sslparser();  // various initialisation functions
void init_basicblock(); // for garbage collection safety

#ifndef NO_CMDLINE_MAIN
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    CommandlineDriver driver;

    // setupCrashHandler(); // Display a backtrace once a serious signal is delivered.

    // Call the various initialisation functions for safe garbage collection
    init_dfa();
    init_sslparser();
    init_basicblock();
    driver.applyCommandline();
    driver.decompile();
    // Boomerang::get()->commandLine(argc, argv)
    return app.exec();
}
#endif

#ifndef NO_NEW_OR_DELETE_OPERATORS
#ifndef NO_GARBAGE_COLLECTOR
/* This makes sure that the garbage collector sees all allocations, even those
        that we can't be bothered collecting, especially standard STL objects */
void *operator new(size_t n) {
#ifdef DONT_COLLECT_STL
    return GC_malloc_uncollectable(n); // Don't collect, but mark
#else
    return GC_malloc(n); // Collect everything
#endif
}

void operator delete(void *p) {
#ifdef DONT_COLLECT_STL
    GC_free(p); // Important to call this if you call GC_malloc_uncollectable
                // #else do nothing!
#endif
}
#endif
#endif
