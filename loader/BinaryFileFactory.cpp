/* File: BinaryFileFactory.cpp
 * Desc: This file contains the implementation of the factory function
 * BinaryFile::getInstanceFor(), and also BinaryFile::Load()
 * 
 * This function determines the type of a binary and loads the appropriate
 * loader class dynamically.
*/

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>			// include before types.h: name collision of NO_ADDRESS and WinSock.h
#endif

#include "BinaryFile.h"
#include "ElfBinaryFile.h"
#include "Win32BinaryFile.h"
#include "PalmBinaryFile.h"
#include "HpSomBinaryFile.h"
#include "ExeBinaryFile.h"
#include "config.h"				// For UNDERSCORE_NEEDED etc

#include <iostream>

BinaryFile *BinaryFileFactory::Load( const char *sName )
{
	BinaryFile *pBF = getInstanceFor( sName );
	if( pBF == NULL ) {
		std::cerr << "unrecognised binary file format.\n";
		return NULL;
	}
	if( pBF->RealLoad( sName ) == 0 ) {
		fprintf( stderr, "Loading '%s' failed\n", sName );
		delete pBF;
		return NULL;
	}
pBF->getTextLimits();
	return pBF;
}

#define TESTMAGIC2(buf,off,a,b)		(buf[off] == a && buf[off+1] == b)
#define TESTMAGIC4(buf,off,a,b,c,d) (buf[off] == a && buf[off+1] == b && \
									 buf[off+2] == c && buf[off+3] == d)

// Declare a pointer to a constructor function; returns a BinaryFile*
typedef BinaryFile* (*constructFcn)();

BinaryFile* BinaryFileFactory::getInstanceFor( const char *sName ) {
	FILE *f;
	unsigned char buf[64];
	std::string libName;
	BinaryFile *res = NULL;

	f = fopen (sName, "r");
	if( f == NULL ) {
		fprintf(stderr, "Unable to open binary file: %s\n", sName );
		return NULL;
	}
	fread (buf, sizeof(buf), 1, f);
	if( TESTMAGIC4(buf,0, '\177','E','L','F') ) {
		/* ELF Binary */
		libName = "ElfBinaryFile";
	} else if( TESTMAGIC2( buf,0, 'M','Z' ) ) { /* DOS-based file */
		int peoff = LMMH(buf[0x3C]);
		if( peoff != 0 && fseek(f, peoff, SEEK_SET) != -1 ) {
			fread( buf, 4, 1, f );
			if( TESTMAGIC4( buf,0, 'P','E',0,0 ) ) {
				/* Win32 Binary */
				libName = "Win32BinaryFile";
			} else if( TESTMAGIC2( buf,0, 'N','E' ) ) {
				/* Win16 / Old OS/2 Binary */
			} else if( TESTMAGIC2( buf,0, 'L','E' ) ) {
				/* Win32 VxD (Linear Executable) or DOS4GW app */
                libName = "DOS4GWBinaryFile";
			} else if( TESTMAGIC2( buf,0, 'L','X' ) ) {
				/* New OS/2 Binary */
			}
		}
		/* Assume MS-DOS Real-mode binary. */
		if( libName.size() == 0 )
			libName = "ExeBinaryFile";
	} else if( TESTMAGIC4( buf,0x3C, 'a','p','p','l' ) ||
			   TESTMAGIC4( buf,0x3C, 'p','a','n','l' ) ) {
		/* PRC Palm-pilot binary */
		libName = "PalmBinaryFile";
    } else if( buf[0] == 0xfe && buf[1] == 0xed && buf[2] == 0xfa && buf[3] == 0xce ) {
        /* Mach-O Mac OS-X binary */
        libName = "MachOBinaryFile";
#ifdef __CYGWIN__
		fprintf(stderr, "Sorry, Cygwin-hosted Boomerang cannot compile the MachOBinaryFile module at present"
			"\n");
		fclose(f);
		return NULL;
#endif
	} else if( buf[0] == 0x02 && buf[2] == 0x01 &&
			   (buf[1] == 0x10 || buf[1] == 0x0B) &&
			   (buf[3] == 0x07 || buf[3] == 0x08 || buf[4] == 0x0B) ) {
		/* HP Som binary (last as it's not really particularly good magic) */
		libName = "HpSomBinaryFile";
	} else {
		fprintf( stderr, "Unrecognised binary file\n" );
		fclose(f);
		return NULL;
	}
	
// Load the specific loader library
#ifndef _WIN32		// Cygwin, Unix/Linux
	libName = std::string("lib/lib") + libName;
#ifdef	__CYGWIN__
	libName += ".dll";		// Cygwin wants .dll, but is otherwise like Unix
#else
#if HOST_OSX
	libName += ".dylib";
#else
	libName += ".so";
#endif
#endif
	dlHandle = dlopen(libName.c_str(), RTLD_LAZY);
	if (dlHandle == NULL) {
		fprintf( stderr, "Could not open dynamic loader library %s\n", libName.c_str());
		fprintf( stderr, "%s\n", dlerror());
		fclose(f);
		return NULL;
	}
	// Use the handle to find the "construct" function
#if UNDERSCORE_NEEDED
#define UNDERSCORE "_"
#else
#define UNDERSCORE
#endif
	constructFcn pFcn = (constructFcn) dlsym(dlHandle, UNDERSCORE "construct");
#else						// Else MSVC, MinGW
	libName += ".dll";		// Example: ElfBinaryFile.dll (same dir as boomerang.exe)
#ifdef __MINGW32__
	libName = "lib/lib" + libName;
#endif
	hModule = LoadLibrary(libName.c_str());
	if(hModule == NULL) {
		int err = GetLastError();
		fprintf( stderr, "Could not open dynamic loader library %s (error #%d)\n", libName.c_str(), err);
		fclose(f);
		return NULL;
	}
	// Use the handle to find the "construct" function
	constructFcn pFcn = (constructFcn) GetProcAddress((HINSTANCE)hModule, "construct");
#endif

	if (pFcn == NULL) {
		fprintf( stderr, "Loader library %s does not have a construct function\n", libName.c_str());
#ifndef _WIN32
		fprintf( stderr, "dlerror returns %s\n", dlerror());
#endif
		fclose(f);
		return NULL;
	}
	// Call the construct function
	res = (*pFcn)();
	fclose(f);
	return res;
}

void BinaryFileFactory::UnLoad() {
#ifdef _WIN32
	FreeLibrary((HINSTANCE)hModule);
#else
	dlclose(dlHandle);					// Especially important for Mac OS X
#endif
}
