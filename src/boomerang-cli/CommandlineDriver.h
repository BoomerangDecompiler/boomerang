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
#include "boomerang-cli/MiniDebugger.h"

#include "boomerang/core/Project.h"

#include <QObject>
#include <QTimer>


class CommandlineDriver : public QObject
{
    Q_OBJECT

public:
    explicit CommandlineDriver(QObject *parent = nullptr);

public:
    /**
     * Apply the list of command line arguments to the Settings.
     * \returns Zero if the binary file can be decompiled, non-zero to exit the program.
     */
    int applyCommandline(const QStringList &args);

    /**
     * Do the whole works - Load, decode, decompile and generate code for a binary file.
     * \ref applyCommandline must be called first.
     * \internal See also m_pathToBinary
     *
     * \returns Zero on success, non-zero on failure.
     */
    int decompile();

    /**
     * Displays a command line and processes the commands entered.
     *
     * \retval 0 stdin was closed.
     * \retval 2 The user typed exit or quit.
     */
    int interactiveMain();

    const Project *getProject() const { return m_project.get(); }

private:
    /**
     * Loads the executable file and decodes it.
     * \param fname The name of the file to load.
     * \param pname How the Prog will be named.
     */
    bool loadAndDecode(const QString &fname, const QString &pname);

    /**
     * The program will be subsequently be loaded, decoded, decompiled and written to a source file.
     * After decompilation the elapsed time is printed to LOG_STREAM().
     *
     * \param fname The name of the file to load.
     * \param pname The name that will be given to the Proc.
     *
     * \return Zero on success, nonzero on faillure.
     */
    int decompile(const QString &fname, const QString &pname);

public slots:
    void onCompilationTimeout();

private:
    std::unique_ptr<Project> m_project;
    std::unique_ptr<Console> m_console;
    std::unique_ptr<MiniDebugger> m_debugger;

    QTimer m_kill_timer;
    int minsToStopAfter = 0;
    QString m_pathToBinary;
};
