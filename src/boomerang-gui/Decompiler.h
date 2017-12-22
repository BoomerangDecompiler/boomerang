#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang/core/Watcher.h"

#include <QObject>
#include <QTableWidget>
#include <QString>


class Module;
class IFrontEnd;
class Prog;
class IBinaryImage;


/**
 *
 */
class Decompiler : public QObject, public IWatcher
{
    Q_OBJECT

public:
    Decompiler()
        : QObject()
        , m_debugging(false)
        , m_waiting(false)
    {}

    /// IWatcher interface
public:
    virtual void alertDecompileDebugPoint(UserProc *p, const char *description) override;
    virtual void alertConsidering(Function *parent, Function *p) override;
    virtual void alertDecompiling(UserProc *p) override;
    virtual void alertNew(Function *p) override;
    virtual void alertRemove(Function *p) override;
    virtual void alertUpdateSignature(Function *p) override;

signals: // Decompiler -> ui

public slots: // ui -> Decompiler
    void loadInputFile(const QString& inputFile, const QString& outputPath);
    void decode();
    void decompile();
    void generateCode();

    void stopWaiting();

    // todo: provide thread-safe access mechanism
public:
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

    std::vector<Address> m_userEntrypoints;
};
