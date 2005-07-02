#include "boomerang.h"
//#define GC_DEBUG 1		// Uncomment to debug the garbage collector
#include "gc.h"

// FIXME: surely not needed here now?
#ifdef WIN32
#include <direct.h>		// For Windows mkdir
#endif
#include <signal.h>

#ifdef SPARC_DEBUG

void segv_handler(int a, siginfo_t *b, void *c)
{
	fprintf(stderr, "Boomerang has encounted a fatal error.\n");

	ucontext_t *uc = (ucontext_t *) c;
#if 0
	fprintf(stderr, "\n*** SIGNAL TRAPPED: SIGNAL %d ***\n", a);
	fprintf(stderr, "si_signo = %d\n", b->si_signo);
	fprintf(stderr, "si_code  = %d\n", b->si_code);
	fprintf(stderr, "si_errno = %d\n", b->si_errno);
	fprintf(stderr, "si_addr  = %p\n", b->si_addr);
	fprintf(stderr, "si_trapno= %p\n", b->si_trapno);
	fprintf(stderr, "si_pc	  = %p\n\n", b->si_pc);

	fprintf(stderr, "stack info:\nsp:	%8x	 size:		%8x\n"
			"flags:		%d\n\n",
			uc->uc_stack.ss_sp, uc->uc_stack.ss_size,
			uc->uc_stack.ss_flags);

	fprintf(stderr, "Registers:\npc: %8x  npc: %8x\n"
			"o0: %8x  o1: %8x  o2: %8x	o3: %8x\n"
			"o4: %8x  o5: %8x  o6: %8x	o7: %8x\n\n"
			"g1: %8x  g2: %8x  g3: %8x	g4: %8x\n"
			"g5: %8x  g6: %8x  g7: %8x\n\n",
			uc->uc_mcontext.gregs[REG_PC],
			uc->uc_mcontext.gregs[REG_nPC],
			uc->uc_mcontext.gregs[REG_O0],
			uc->uc_mcontext.gregs[REG_O1],
			uc->uc_mcontext.gregs[REG_O2],
			uc->uc_mcontext.gregs[REG_O3],
			uc->uc_mcontext.gregs[REG_O4],
			uc->uc_mcontext.gregs[REG_O5],
			uc->uc_mcontext.gregs[REG_O6],
			uc->uc_mcontext.gregs[REG_O7],
			uc->uc_mcontext.gregs[REG_G1],
			uc->uc_mcontext.gregs[REG_G2],
			uc->uc_mcontext.gregs[REG_G3],
			uc->uc_mcontext.gregs[REG_G4],
			uc->uc_mcontext.gregs[REG_G5],
			uc->uc_mcontext.gregs[REG_G6],
			uc->uc_mcontext.gregs[REG_G7]);

	fprintf(stderr, "stack: ");
	for (int i = 0; i < 100; i++) {
	   unsigned int *sp = (unsigned int*)uc->uc_mcontext.gregs[REG_O6];
	   fprintf(stderr, "(%08X) %08X %08X %08X %08X", sp+i*4, sp[i*4], sp[i*4+1], sp[i*4+2], sp[i*4+3]);
	   fprintf(stderr, "\n		 ");
	}
#endif

	fprintf(stderr, "\napproximate stack trace:\n");
	for (int i = 0; i < 100; i++) {
		unsigned int *sp = (unsigned int*)uc->uc_mcontext.gregs[REG_O6];
		if (sp[i] - (unsigned int)(sp+i) < 100)
			fprintf(stderr, "%08X\n", sp[++i]);
	}
   exit(0);
}

int main(int argc, const char* argv[]) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction = segv_handler;
	act.sa_flags = (SA_SIGINFO | SA_ONSTACK);

	sigaction(SIGSEGV, &act, NULL);
#else
int main(int argc, const char* argv[]) {
#endif
	return Boomerang::get()->commandLine(argc, argv);
}

/* This makes sure that the garbage collector sees all allocations, even those
	that we can't be bothered collecting, especially standard STL objects */
void* operator new(size_t n) {
#ifdef DONT_COLLECT_STL
	return GC_malloc_uncollectable(n);	// Don't collect, but mark
#else
	return GC_malloc(n);				// Collect everything
#endif
}

void operator delete(void* p) {
#ifdef DONT_COLLECT_STL
	GC_free(p); // Important to call this if you call GC_malloc_uncollectable
	// #else do nothing!
#endif
}

