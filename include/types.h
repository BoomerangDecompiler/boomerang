// Machine types
typedef unsigned char       Byte;      /* 8 bits */
typedef unsigned short      SWord;     /* 16 bits */
typedef unsigned int        DWord;     /* 32 bits */
typedef unsigned int        dword;     /* 32 bits */
typedef unsigned int        Word;      /* 32 bits */
typedef unsigned int        ADDRESS;   /* 32-bit unsigned */


#define NO_ADDRESS ((ADDRESS)-1)  // For invalid ADDRESSes

#ifndef _MSC_VER
typedef long unsigned long QWord;	// 64 bits
#else
typedef unsigned __int64   QWord;		
#endif
