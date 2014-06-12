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
  *           Also contains main(), so it can be the only file different between boomerang and bigtest
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

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    CommandlineDriver driver;

    // setupCrashHandler(); // Display a backtrace once a serious signal is delivered.

    // Call the various initialisation functions for safe garbage collection
    init_dfa();
    init_sslparser();
    init_basicblock();
    driver.applyCommandline();
    return driver.decompile();
}
