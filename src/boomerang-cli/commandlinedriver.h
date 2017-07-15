#pragma once

#include <QObject>
#include <QTimer>
#include <QThread>


class DecompilationThread : public QThread
{
    Q_OBJECT

public:
    void run() override;

    void setDecompiled(const QString value) { m_decompiled = value; }
    int resCode() { return m_result; }

private:
    QString m_decompiled;
    int m_result = 0;
};


class CommandlineDriver : public QObject
{
    Q_OBJECT

public:
    explicit CommandlineDriver(QObject *parent = nullptr);
    int applyCommandline(const QStringList& args);
    int decompile();
    int console();

public slots:
    void onCompilationTimeout();

private:
    DecompilationThread m_thread;
    QTimer m_kill_timer;
    int minsToStopAfter = 0;

};
