/*==============================================================================
 * FILE:       scope.h
 * OVERVIEW:   Provides the definition for the Scope class.
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Apr 02 - Trent: Created
 */

#ifndef __SCOPE_H_
#define __SCOPE_H_

#include <fstream>      // For ostream, cout etc
#include <stdio.h>      // For sprintf
#include <list>

class Decl;

/*============================================================================== 
 * Scope is a scope class, it holds declarations which may be attached to 
 * different scopes.  BlockStmt extends this class which (apart possibly for 
 * files) provides all the scoping rules of the C language.
 *============================================================================*/
class Scope {
   list<Decl *> *decls;
public:
   Scope() : decls(NULL) { }
   Scope(list<Decl *> *ld) : decls(ld) { }
   list<Decl *> *getDecls() { return decls; }
   void addDecl(Decl *d) { decls->push_back(d); }
   void    print(ostream& os = cout);
};

#endif // __SCOPE_H__
