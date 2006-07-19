/*
 * Copyright (C) 2006, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */


/*
 * This file contains a truely diabolical hack to make Qt4 threads work 
 * nicely with the Boehm garbage collector.  It works by exporting a 
 * pthread_create symbol that Qt4 links to.  If the start routine is not
 * GC_start_routine, it calls the garbage collector to create the thread,
 * otherwise it dynamically loads libpthread and calls the real 
 * pthread_create.
 *
 * This prevents us from having to rebuild Qt4 with 
 *
 * #define GC_THREADS
 * #include <gc/gc.h>
 *
 * in qthread_unix.cpp, which would be the sane way to fix this problem.
 *
 *  - trentw
 */

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

extern "C" void *GC_start_routine(void * arg);

typedef void *pthread_create_type(void *a, void *b, void *c, void *d);

extern "C" void *GC_pthread_create(void *, void *, void *, void *);

void *pthread_lib = NULL;
pthread_create_type *orig = NULL;

extern "C" void *pthread_create(void *a, void *b, void *c, void *d)
{
	if (c != (void*)GC_start_routine)
		return GC_pthread_create(a, b, c, d);
	else {
		if (pthread_lib == NULL) {
			pthread_lib = dlopen("libpthread.so.0", RTLD_LAZY);
			if (pthread_lib == NULL) {
				printf("cannot dynamically open pthreads %s.\n", dlerror());
				exit(1);
			}
			orig = (pthread_create_type*)dlsym(pthread_lib, "pthread_create");
			if (orig == NULL) {
				printf("cannot find symbol pthread_create %s.\n", dlerror());
				exit(1);
			}
		}
		return (*orig)(a, b, c, d);
	}
}


