#pragma once
#include <QThread>
#include <QString>
#include <QTableWidget>

#undef NO_ADDRESS
#include "boom_base/boomerang.h"

class MainWindow;
class FrontEnd;
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
		, Debugging(false)
		, Waiting(false) {}

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

	void setDebugging(bool d) { Debugging = d; }
	void setUseDFTA(bool d);
	void setNoDecodeChildren(bool d);

	void addEntryPoint(ADDRESS a, const char *nam);
	void removeEntryPoint(ADDRESS a);

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
	void newUserProc(const QString& name, ADDRESS addr);
	void newLibProc(const QString& name, const QString& params);
	void removeUserProc(const QString& name, ADDRESS addr);
	void removeLibProc(const QString& name);
	void newCluster(const QString& name);
	void newProcInCluster(const QString& name, const QString& cluster);
	void newEntrypoint(ADDRESS addr, const QString& name);
	void newSection(const QString& name, ADDRESS start, ADDRESS end);

	void machineType(const QString& machine);

	void debuggingPoint(const QString& name, const QString& description);

protected:
	bool Debugging, Waiting;

	FrontEnd *fe;
	Prog *prog;
	IBinaryImage *Image;
	QString filename;

	const char *procStatus(UserProc *p);
	void emitClusterAndChildren(Module *root);

	std::vector<ADDRESS> user_entrypoints;
};

class DecompilerThread : public QThread
{
	Q_OBJECT

public:
	DecompilerThread()
		: QThread()
		, Parent(nullptr)
    {}

	Decompiler *getDecompiler();

protected:
	void run();

	Decompiler *Parent;
};
