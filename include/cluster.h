/*
 * Copyright (C) 2004, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       cluster.h
 * OVERVIEW:   Definition of the classes that describe a Cluster, a grouping
 * 	       of functions irrespective of relationship.  For example, the
 * 	       Object Oriented Programming concept of a Class is a Cluster. 
 * 	       Clusters can contain other Clusters to form a tree.
 *============================================================================*/

/*
 * $Revision$
 * 03 May 04 - Trent: Created
 */

#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include "memo.h"

class XMLProgParser;
class Cluster;

class Cluster : public Memoisable
{
protected:
    std::string name;
    std::vector<Cluster*> children;
    Cluster *parent;
    std::ofstream out;
	std::string stream_ext;

public:
    Cluster() : name(""), parent(NULL) { }
    Cluster(const char *name) : name(name), parent(NULL) { }
    const char *getName() { return name.c_str(); }
    void setName(const char *nam) { name = nam; }
    unsigned int getNumChildren() { return children.size(); }
    Cluster *getChild(int n) { return children[n]; }
    void addChild(Cluster *n);
    void removeChild(Cluster *n);
    Cluster *getParent() { return parent; }
    bool hasChildren() { return children.size() > 0; }
    void openStream(const char *ext);
    void openStreams(const char *ext);
    void closeStreams();
    std::ofstream &getStream() { return out; }
    const char *makeDirs();
    const char *getOutPath(const char *ext);
    Cluster *find(const char *nam);

	virtual Memo *makeMemo(int mId);
	virtual void readMemo(Memo *m, bool dec);

    void printTree(std::ostream &out);
protected:

    friend class XMLProgParser;
};

#endif /*__CLUSTER_H__*/

