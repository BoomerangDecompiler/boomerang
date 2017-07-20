#pragma once
#include <QThread>
#include <QString>
#include <QTableWidget>

#include "boomerang/core/Boomerang.h"

class MainWindow;
class IFrontEnd;
class Function;
class UserProc;
class Prog;
class Module;
class QTableWidget;
class QObject;

class Decompiler : public QObject, public Watcher
{
    Q_OBJECT

public:
    Decompiler()
        : QObject()
        , m_debugging(false)
        , m_waiting(false)
    {}

    virtual void alertDecompileDebugPoint(UserProc *p, const char *description) override;
    virtual void alertConsidering(Function *parent, Function *p) override;
    virtual void alertDecompiling(UserProc *p) override;
    virtual void alertNew(Function *p) override;
    virtual void alertRemove(Function *p) override;
    virtual void alertUpdateSignature(Function *p) override;

    bool getRtlForProc(const QString& name, QString& rtl);
    QString getSigFile(const QString& name);
    QString getClusterFile(const QString& name);
    void renameProc(const QString& oldName, const QString& newName);
    void rereadLibSignatures();
    void getCompoundMembers(const QString& name, QTableWidget *tbl);

    void setDebugging(bool d) { m_debugging = d; }
    void setUseDFTA(bool d);
    void setNoDecodeChildren(bool d);

    void addEntryPoint(Address a, const char *nam);
    void removeEntryPoint(Address a);

public slots:
    void changeInputFile(const QString& f);
    void changeOutputPath(const QString& path);
    void load();
    void decode();
    void decompile();
    void generateCode();
    void stopWaiting();

signals:
    void loading();
    void decoding();
    void decompiling();
    void generatingCode();
    void loadCompleted();
    void decodeCompleted();
    void decompileCompleted();
    void generateCodeCompleted();

    void consideringProc(const QString& parent, const QString& name);
    void decompilingProc(const QString& name);
    void newUserProc(const QString& name, Address addr);
    void newLibProc(const QString& name, const QString& params);
    void removeUserProc(const QString& name, Address addr);
    void removeLibProc(const QString& name);
    void newCluster(const QString& name);
    void newProcInCluster(const QString& name, const QString& cluster);
    void newEntrypoint(Address addr, const QString& name);
    void newSection(const QString& name, Address start, Address end);

    void machineType(const QString& machine);

    void debuggingPoint(const QString& name, const QString& description);

protected:
    void emitClusterAndChildren(Module *root);
    const char *getProcStatus(UserProc *p);

protected:
    bool m_debugging;
    bool m_waiting;

    IFrontEnd *m_fe;
    Prog *m_prog;
    IBinaryImage *m_image;
    QString m_filename;

    std::vector<Address> m_userEntrypoints;
};


class DecompilerThread : public QThread
{
    Q_OBJECT

public:
    DecompilerThread()
        : QThread()
        , m_parent(nullptr)
    {}

    Decompiler *getDecompiler();

protected:
    void run() override;

    Decompiler *m_parent;
};
