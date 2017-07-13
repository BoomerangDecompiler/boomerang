#pragma once

/// Needed by both signature.h and frontend.h
enum class Platform
{
	GENERIC = 0,
	PENTIUM,
	SPARC,
	M68K,
	PARISC,
	PPC,
	MIPS,
	ST20,
};


enum class CallConv
{
	INVALID = 0,
	C,        ///< Standard C, no callee pop
	Pascal,   ///< callee pop
	ThisCall, ///< MSVC "thiscall": one parameter in register ecx
	FastCall, ///< MSVC fastcall convention ECX,EDX,stack, callee pop
};
