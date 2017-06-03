#pragma once
#include <QObject>
#include <QTimer>
#include <QThread>
class DecompilationThread : public QThread
{
	Q_OBJECT
	QString m_decompiled;
	int Result = 0;

public:
	void run() override;

	void setDecompiled(const QString value) { m_decompiled = value; }
	int resCode() { return Result; }
};

class CommandlineDriver : public QObject
{
	Q_OBJECT
	DecompilationThread m_thread;
	QTimer m_kill_timer;
	int minsToStopAfter = 0;

public:
	explicit CommandlineDriver(QObject *parent = 0);
	int applyCommandline(const QStringList& args);
	int decompile();
	int console();

public slots:
	void onCompilationTimeout();
};
