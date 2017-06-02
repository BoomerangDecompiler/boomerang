#pragma once

// Needed by both signature.h and frontend.h
enum Platform
{
	PLAT_PENTIUM,
	PLAT_SPARC,
	PLAT_M68K,
	PLAT_PARISC,
	PLAT_PPC,
	PLAT_MIPS,
	PLAT_ST20,
	PLAT_GENERIC
};

enum callconv
{
	CONV_C,        // Standard C, no callee pop
	CONV_PASCAL,   // callee pop
	CONV_THISCALL, // MSVC "thiscall": one parameter in register ecx
	CONV_FASTCALL, // MSVC fastcall convention ECX,EDX,stack, callee pop
	CONV_NONE
};
