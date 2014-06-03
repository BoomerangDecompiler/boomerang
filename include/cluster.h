/*
 * Copyright (C) 2004, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       cluster.h
  *    Definition of the classes that describe a Cluster, a grouping
  * of functions irrespective of relationship.  For example, the
  * Object Oriented Programming concept of a Class is a Cluster.
  * Clusters can contain other Clusters to form a tree.
  ******************************************************************************/

#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#include "memo.h"

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <QtCore/QTextStream>
#include <QtCore/QFile>

class XMLProgParser;
class Cluster;

class Cluster {
  protected:
    QString Name;
    std::vector<Cluster *> Children;
    Cluster *Parent = nullptr;
    QFile out;
    QTextStream strm;
    std::string stream_ext;

  public:
    Cluster();
    Cluster(const QString &_name);
    virtual ~Cluster() {}
    QString getName() { return Name; }
    void setName(const QString &nam) { Name = nam; }
    size_t getNumChildren() { return Children.size(); }
    Cluster *getChild(size_t n) { return Children[n]; }
    void addChild(Cluster *n);
    void removeChild(Cluster *n);
    Cluster *getParent() { return Parent; }
    bool hasChildren() { return Children.size() > 0; }
    void openStream(const char *ext);
    void openStreams(const char *ext);
    void closeStreams();
    QTextStream &getStream() { return strm; }
    QString makeDirs();
    QString getOutPath(const char *ext);
    Cluster *find(const QString &nam);
    virtual bool isAggregate() { return false; }
    void printTree(std::ostream &out);

  protected:
    friend class XMLProgParser;
};

class Module : public Cluster {
  public:
    Module(const char *name) : Cluster(name) {}
};

class Class : public Cluster {
  protected:
    CompoundType *Type;

  public:
    Class(const char *name) : Cluster(name) { Type = new CompoundType(); }

    // A Class tends to be aggregated into the parent Module,
    // this isn't the case with Java, but hey, we're not doing that yet.
    virtual bool isAggregate() { return true; }
};

#endif /*__CLUSTER_H__*/
