
#include <QThread>
#undef NO_ADDRESS
#include "boomerang.h"

class MainWindow;
class FrontEnd;
class Proc;
class UserProc;
class Prog;
class Cluster;
class QTableWidget;

class Decompiler : public QObject, public Watcher
{
	Q_OBJECT

public:
	Decompiler() : QObject(), debugging(false), waiting(false) { }

	virtual void alert_decompile_debug_point(UserProc *p, const char *description);
	virtual void alert_considering(Proc *parent, Proc *p);
	virtual void alert_decompiling(UserProc *p);
	virtual void alert_new(Proc *p);
	virtual void alert_update_signature(Proc *p);

	bool getRtlForProc(const QString &name, QString &rtl);
	const char *getSigFile(const QString &name);
	const char *getClusterFile(const QString &name);
	void renameProc(const QString &oldName, const QString &newName);
	void rereadLibSignatures();
	void getCompoundMembers(const QString &name, QTableWidget *tbl);

	void setDebugging(bool d) { debugging = d; }

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
	void newUserProc(const QString &name, unsigned int addr);
	void newLibProc(const QString &name, const QString &params);
	void newCluster(const QString &name);
	void newProcInCluster(const QString &name, const QString &cluster);
	void newEntrypoint(unsigned int addr, const QString &name);
	void newSection(const QString &name, unsigned int start, unsigned int end);

	void machineType(const QString &machine);

	void debuggingPoint(const QString &name, const QString &description);

protected:

	bool debugging, waiting;

	FrontEnd *fe;
	Prog *prog;

	QString filename;

	const char *procStatus(UserProc *p);
	void emitClusterAndChildren(Cluster *root);
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
