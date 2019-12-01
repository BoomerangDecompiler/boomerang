#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang/core/Project.h"
#include "boomerang/core/Watcher.h"

#include <QObject>
#include <QString>
#include <QTableWidget>


class Module;
class IFrontEnd;
class Prog;
class BinaryFile;


Q_DECLARE_METATYPE(Address)


/**
 * Interface between libboomerang and the GUI.
 */
class Decompiler : public QObject, public IWatcher
{
    Q_OBJECT

public:
    Decompiler();
    ~Decompiler();

    /// IWatcher interface
public:
    void onDecompileDebugPoint(UserProc *proc, const char *description) override;
    void onFunctionDiscovered(Function *function) override;
    void onDecompileInProgress(UserProc *function) override;
    void onFunctionCreated(Function *function) override;
    void onFunctionRemoved(Function *function) override;
    void onSignatureUpdated(Function *function) override;

signals: // Decompiler -> ui
    void loadingStarted();
    void decodingStarted();
    void decompilingStarted();
    void generatingCodeStarted();

    void loadCompleted();
    void decodeCompleted();
    void decompileCompleted();
    void generateCodeCompleted();

    void procDiscovered(const QString &callerName, const QString &procName);
    void procDecompileStarted(const QString &procName);

    void userProcCreated(const QString &name, Address entryAddr);
    void libProcCreated(const QString &name, const QString &params);
    void userProcRemoved(const QString &name, Address entryAddr);
    void libProcRemoved(const QString &name);
    void moduleCreated(const QString &name);
    void functionAddedToModule(const QString &functionName, const QString &moduleName);
    void entryPointAdded(Address entryAddr, const QString &name);
    void sectionAdded(const QString &sectionName, Address start, Address end);

    void machineTypeChanged(const QString &machine);

    void debugPointHit(const QString &name, const QString &description);

public slots: // ui -> Decompiler
    void loadInputFile(const QString &inputFile, const QString &outputPath);
    void decode();
    void decompile();
    void generateCode();

    void stopWaiting();
    void rereadLibSignatures();

    void addEntryPoint(Address entryAddr, const QString &name);
    void removeEntryPoint(Address entryAddr);

    // todo: provide thread-safe access mechanism
public:
    bool getRTLForProc(const QString &name, QString &rtl);
    QString getSigFilePath(const QString &name);
    QString getClusterFile(const QString &name);
    void renameProc(const QString &oldName, const QString &newName);
    void getCompoundMembers(const QString &name, QTableWidget *tbl);

    void setDebugEnabled(bool debug) { m_debugging = debug; }
    Project *getProject() { return &m_project; }

private:
    /// After code generation, update the list of modules
    void moduleAndChildrenUpdated(Module *root);

protected:
    bool m_debugging = false;
    bool m_waiting   = false;

    Project m_project;

    std::vector<Address> m_userEntrypoints;
};
