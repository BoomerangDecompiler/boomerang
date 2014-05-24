
#include <QThread>
#include <QString>
#include <QTableWidget>

#undef NO_ADDRESS
#include "../include/boomerang.h"

class MainWindow;
class FrontEnd;
class Proc;
class UserProc;
class Prog;
class Cluster;
class QTableWidget;
class QObject;

class Decompiler : public QObject, public Watcher
{
    Q_OBJECT

public:
    Decompiler() : QObject(), debugging(false), waiting(false) { }

    virtual void alert_decompile_debug_point(UserProc *p, const char *description) override;
    virtual void alert_considering(Proc *parent, Proc *p) override;
    virtual void alert_decompiling(UserProc *p) override;
    virtual void alert_new(Proc *p) override;
    virtual void alertRemove(Proc *p) override;
    virtual void alert_update_signature(Proc *p) override;

    bool getRtlForProc(const QString &name, QString &rtl);
    const char *getSigFile(const QString &name);
    const char *getClusterFile(const QString &name);
    void renameProc(const QString &oldName, const QString &newName);
    void rereadLibSignatures();
    void getCompoundMembers(const QString &name, QTableWidget *tbl);

    void setDebugging(bool d) { debugging = d; }
    void setUseDFTA(bool d);
    void setNoDecodeChildren(bool d);

    void addEntryPoint(ADDRESS a, const char *nam);
    void removeEntryPoint(ADDRESS a);

public slots:
    void changeInputFile(const QString &f);
    void changeOutputPath(const QString &path);
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

    void consideringProc(const QString &parent, const QString &name);
    void decompilingProc(const QString &name);
    void newUserProc(const QString &name, ADDRESS addr);
    void newLibProc(const QString &name, const QString &params);
    void removeUserProc(const QString &name, ADDRESS addr);
    void removeLibProc(const QString &name);
    void newCluster(const QString &name);
    void newProcInCluster(const QString &name, const QString &cluster);
    void newEntrypoint(ADDRESS addr, const QString &name);
    void newSection(const QString &name, ADDRESS start, ADDRESS end);

    void machineType(const QString &machine);

    void debuggingPoint(const QString &name, const QString &description);

protected:

    bool debugging, waiting;

    FrontEnd *fe;
    Prog *prog;

    QString filename;

    const char *procStatus(UserProc *p);
    void emitClusterAndChildren(Cluster *root);

    std::vector<ADDRESS> user_entrypoints;
};

class DecompilerThread : public QThread
{
    Q_OBJECT

public:
    DecompilerThread() : QThread(), decompiler(NULL) { }

    Decompiler *getDecompiler();

protected:
    void run();

    Decompiler *decompiler;
};
