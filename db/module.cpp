#include "module.h"

#include "boomerang.h"

#include <QDir>
#include <QString>

Module::Module() { strm.setDevice(&out); }

Module::Module(const QString &_name) : Name(_name) { strm.setDevice(&out); }

size_t Module::getNumChildren() { return Children.size(); }

Module *Module::getChild(size_t n) { return Children[n]; }

void Module::addChild(Module *n) {
    if (n->Parent)
        n->Parent->removeChild(n);
    Children.push_back(n);
    n->Parent = this;
}

void Module::removeChild(Module *n) {
    auto it = Children.begin();
    for (; it != Children.end(); it++)
        if (*it == n)
            break;
    assert(it != Children.end());
    Children.erase(it);
}

Module *Module::getParent() { return Parent; }

bool Module::hasChildren() { return Children.size() > 0; }

void Module::openStream(const char *ext) {
    if (out.isOpen())
        return;
    out.setFileName(getOutPath(ext));
    out.open(QFile::WriteOnly | QFile::Text);
    stream_ext = ext;
}

void Module::openStreams(const char *ext) {
    openStream(ext);
    for (Module *child : Children)
        child->openStreams(ext);
}

void Module::closeStreams() {
    if (out.isOpen()) {
        out.close();
    }
    for (Module *child : Children)
        child->closeStreams();
}

QString Module::makeDirs() {
    QString path;
    if (Parent)
        path = Parent->makeDirs();
    else
        path = Boomerang::get()->getOutputPath();
    QDir dr(path);
    if (getNumChildren() > 0 || Parent == nullptr) {
        dr.mkpath(Name);
        dr.cd(Name);
    }
    return dr.absolutePath();
}

QString Module::getOutPath(const char *ext) {
    QString basedir = makeDirs();
    QDir dr(basedir);
    return dr.absoluteFilePath(Name + "." + ext);
}

Module *Module::find(const QString &nam) {
    if (Name == nam)
        return this;
    for (Module *child : Children) {
        Module *c = child->find(nam);
        if (c)
            return c;
    }
    return nullptr;
}
/**
 * Prints a tree graph.
 */
void Module::printTree(QTextStream &ostr) {
    ostr << "\t\t" << Name << "\n";
    for (Module *elem : Children)
        elem->printTree(ostr);
}
