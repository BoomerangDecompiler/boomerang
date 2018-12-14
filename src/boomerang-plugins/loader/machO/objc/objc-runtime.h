/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 *    objc-runtime.h
 *    Copyright 1988-1996, NeXT Software, Inc.
 */

#ifndef _OBJC_RUNTIME_H_
#define _OBJC_RUNTIME_H_

// #import <stdarg.h>
#include "objc.h"
#include "objc-class.h"

typedef struct objc_symtab *Symtab;

struct objc_symtab
{
    uint32_t       sel_ref_cnt;
    SEL            *refs;
    unsigned short cls_def_cnt;
    unsigned short cat_def_cnt;
    uint32_t       defs[1]; /* variable size */
};

// typedef struct objc_module *Module;

struct objc_module
{
    uint32_t version;
    uint32_t size;
    uint32_t name;
    uint32_t symtab;
};

struct objc_super
{
    _id    receiver;
    // Class class;
    Class pclass;
};

/* kernel operations */

OBJC_EXPORT _id objc_getClass(const char *name);
OBJC_EXPORT _id objc_getMetaClass(const char *name);
OBJC_EXPORT _id objc_msgSend(_id self, SEL op, ...);

#if defined(WINNT) || defined(__cplusplus)
// The compiler on NT is broken when dealing with structure-returns.
// Help out the compiler group by tweaking the prototype.
OBJC_EXPORT _id objc_msgSend_stret(_id self, SEL op, ...);

#else
OBJC_EXPORT void objc_msgSend_stret(void *stretAddr, id self, SEL op, ...);

#endif
OBJC_EXPORT _id objc_msgSendSuper(struct objc_super *super, SEL op, ...);

#if defined(WINNT) || defined(__cplusplus)
// The compiler on NT is broken when dealing with structure-returns.
// Help out the compiler group by tweaking the prototype.
OBJC_EXPORT _id objc_msgSendSuper_stret(struct objc_super *super, SEL op, ...);

#else
OBJC_EXPORT void objc_msgSendSuper_stret(void *stretAddr, struct objc_super *super, SEL op, ...);

#endif

/* forwarding operations */

OBJC_EXPORT _id objc_msgSendv(_id self, SEL op, unsigned arg_size, marg_list arg_frame);
OBJC_EXPORT void objc_msgSendv_stret(void *stretAddr, _id self, SEL op, unsigned arg_size, marg_list arg_frame);

/*
 *  getting all the classes in the application...
 *
 *  int objc_getClassList(buffer, bufferLen)
 *  classes is an array of Class values (which are pointers)
 *      which will be filled by the function; if this
 *      argument is nullptr, no copying is done, only the
 *      return value is returned
 *  bufferLen is the number of Class values the given buffer
 *      can hold; if the buffer is not large enough to
 *      hold all the classes, the buffer is filled to
 *      the indicated capacity with some arbitrary subset
 *      of the known classes, which could be different
 *      from call to call
 *  returns the number of classes, which is the number put
 *      in the buffer if the buffer was large enough,
 *      or the length the buffer should have been
 *
 *  int numClasses = 0, newNumClasses = objc_getClassList(nullptr, 0);
 *  Class *classes = nullptr;
 *  while (numClasses < newNumClasses) {
 *      numClasses = newNumClasses;
 *      classes = realloc(classes, sizeof(Class) * numClasses);
 *      newNumClasses = objc_getClassList(classes, numClasses);
 *  }
 *  // now, can use the classes list; if nullptr, there are no classes
 *  free(classes);
 *
 */
OBJC_EXPORT int objc_getClassList(Class *buffer, int bufferLen);

#define OBSOLETE_OBJC_GETCLASSES    1
#if OBSOLETE_OBJC_GETCLASSES
OBJC_EXPORT void *objc_getClasses();

#endif

OBJC_EXPORT _id objc_lookUpClass(const char *name);
OBJC_EXPORT void objc_addClass(Class myClass);

/* customizing the error handling for objc_getClass/objc_getMetaClass */

OBJC_EXPORT void objc_setClassHandler(int (*)(const char *));

/* Making the Objective-C runtime thread safe. */
OBJC_EXPORT void objc_setMultithreaded(BOOL flag);

/* overriding the default object allocation and error handling routines */

OBJC_EXPORT _id (*_alloc)(Class, unsigned int);
OBJC_EXPORT _id (*_copy)(_id, unsigned int);
OBJC_EXPORT _id (*_realloc)(_id, unsigned int);
OBJC_EXPORT _id (*_dealloc)(_id);
OBJC_EXPORT _id (*_zoneAlloc)(Class, unsigned int, void *);
OBJC_EXPORT _id (*_zoneRealloc)(_id, unsigned int, void *);
OBJC_EXPORT _id (*_zoneCopy)(_id, unsigned int, void *);

OBJC_EXPORT void (*_error)(_id, const char *, va_list);

#endif /* _OBJC_RUNTIME_H_ */
