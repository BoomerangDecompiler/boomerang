#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang-cli/Console.h"

#include <QObject>
#include <QTimer>
#include <QThread>


class DecompilationThread : public QThread
{
    Q_OBJECT

public:
    void run() override;

    void setPathToBinary(const QString value)
    {
        m_pathToBinary = value;
    }

    int resCode() { return m_result; }

private:
    QString m_pathToBinary;
    int m_result = 0;
};


class CommandlineDriver : public QObject
{
    Q_OBJECT

public:
    explicit CommandlineDriver(QObject *parent = nullptr);

public:
    int applyCommandline(const QStringList& args);
    int decompile();

    /**
     * Displays a command line and processes the commands entered.
     *
     * \retval 0 stdin was closed.
     * \retval 2 The user typed exit or quit.
     */
    int interactiveMain();

public slots:
    void onCompilationTimeout();

private:
    Console m_console;
    DecompilationThread m_thread;
    QTimer m_kill_timer;
    int minsToStopAfter = 0;
};
