#include "boomerang.h"
#include "gc.h"

int main(int argc, const char* argv[]) {
	return Boomerang::get()->commandLine(argc, argv);
}

/* This makes sure that the garbage collector sees all allocations, even those
    that we can't be bothered collecting, especially standard STL objects */
void* operator new(size_t n) {
#ifdef DONT_COLLECT_STL
    return GC_malloc_uncollectable(n);  // Don't collect, but mark
#else
    return GC_malloc(n);                // Collect everything
#endif
}

void operator delete(void* p) {
#ifdef DONT_COLLECT_STL
    GC_free(p); // Important to call this if you call GC_malloc_uncollectable
    // #else do nothing!
#endif
}

