/*
 * types.h: some often used basic type definitions
 * $Revision$
 */
#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

// Machine types
typedef unsigned char		Byte;		/* 8 bits */
typedef unsigned short		SWord;		/* 16 bits */
typedef unsigned int			DWord;		/* 32 bits */
typedef unsigned int			dword;		/* 32 bits */
typedef unsigned int			Word;		/* 32 bits */
typedef uintptr_t					ADDRESS;	/* pointer. size depends on platform */


#define STD_SIZE	32					// Standard size
// Note: there is a known name collision with NO_ADDRESS in WinSock.h
#ifdef NO_ADDRESS
#undef NO_ADDRESS
#endif
#define NO_ADDRESS ((ADDRESS)-1)		// For invalid ADDRESSes

#ifndef _MSC_VER
typedef long unsigned long QWord;		// 64 bits
#else
typedef unsigned __int64   QWord;		
#endif

#if defined(_MSC_VER)
#pragma warning(disable:4390)
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1200
// For MSVC 5 or 6: warning about debug into truncated to 255 chars
#pragma warning(disable:4786)
#endif

#endif	// #ifndef __TYPES_H__
