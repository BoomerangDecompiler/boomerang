/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************//**
 * \file       syself.h
 * OVERVIEW:   This file contains the various system includes and the like
 *             needed to load elf binary files.
 *============================================================================*/

/*
 * $Revision$
 * 11 May 01 - Nathan: Initial version
 */

#ifndef __SYSELF_H_
#define __SYSELF_H_

#include "config.h"

// ELF Support
#ifdef HAVE_ELF_H
#include <elf.h>
#endif
#ifdef HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h> 
#endif
#ifdef HAVE_LIBELF_H
#include <libelf.h>
#endif
#ifdef HAVE_SYS_ELF_SPARC_H
#include <sys/elf_SPARC.h>
#endif
#ifdef HAVE_LINK_H
#include <link.h>
#endif
#ifdef HAVE_SYS_LINK_H
#include <sys/link.h>
#endif
#ifdef HAVE_SYS_AUXV_H
#include <sys/auxv.h>
#endif
#ifndef HAVE_ELF32_VERSYM
typedef Elf32_Half Elf32_Versym;
#endif

/* Lots of weirdness to deal with slightly different symbols between systems */
#ifndef AT_SUN_EXECNAME
#define AT_SUN_EXECNAME 2014
#endif

#ifndef EM_PARISC
#ifdef EM_PA_RISC
#define EM_PARISC EM_PA_RISC
#endif
#endif

/* Use different symbols here because (lord help me) solaris defines
 * AT_UID and AT_GID to be something quite different. These aren't
 * (afaik) linux specific though
 */
#define AT_LNX_UID          11              /* Real uid */
#define AT_LNX_EUID         12              /* Effective uid */
#define AT_LNX_GID          13              /* Real gid */
#define AT_LNX_EGID         14              /* Effective gid */

#ifndef AT_SUN_UID
#define AT_SUN_UID    2000              /* As above, but for solaris... */
#define AT_SUN_RUID   2001
#define AT_SUN_GID    2002
#define AT_SUN_RGID   2003
#endif

#endif /* !__SYSELF_H_ */
