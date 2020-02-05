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


#include "boomerang/util/Address.h"

#include <QDir>
#include <QString>

#include <map>
#include <vector>


/**
 * Settings that affect decompilation and output behaviour.
 */
class BOOMERANG_API Settings
{
public:
    Settings();

public:
    void setWorkingDirectory(const QString &directoryPath);
    void setDataDirectory(const QString &directoryPath);
    void setPluginDirectory(const QString &directoryPath);
    void setOutputDirectory(const QString &directoryPath);

    /// Get the path where the boomerang executable is run from.
    QDir getWorkingDirectory() const { return m_workingDirectory; }

    /// Get the path of the data directory where ssl files, signatures etc. are stored.
    QDir getDataDirectory() const { return m_dataDirectory; }

    /// Get the path to the plugin directory
    QDir getPluginDirectory() const { return m_pluginDirectory; }

    /// Get the path where the decompiled files should be put
    QDir getOutputDirectory() const { return m_outputDirectory; }

public:
    // Command line flags
    bool verboseOutput       = false;
    bool debugSwitch         = false;
    bool debugLiveness       = false;
    bool debugTA             = false;
    bool debugDecoder        = false;
    bool debugProof          = false;
    bool debugUnused         = false;
    bool printRTLs           = false;
    bool removeNull          = true;
    bool useLocals           = true;
    bool removeLabels        = true;
    bool useDataflow         = true;
    bool stopBeforeDecompile = false;
    bool traceDecoder        = false;

    /// The file in which the dotty graph is saved
    QString dotFile;
    bool usePromotion   = true;
    bool debugGen       = false;
    bool nameParameters = true;

    /// When true, attempt to decode main, all children, and all procs.
    /// \a decodeMain is set when there are no -e or -E switches given
    bool decodeMain        = true;
    bool removeReturns     = true;
    bool decodeThruIndCall = false;
    bool decodeChildren    = true;
    bool useProof          = true;
    bool changeSignatures  = true;
    bool useTypeAnalysis   = true;
    int propMaxDepth       = 3; ///< Max depth of exp that'll be propagated to more than one dest
    bool generateCallGraph = false;
    bool generateSymbols   = false;
    bool useGlobals        = true;
    bool assumeABI         = false; ///< Assume ABI compliance

    QString replayFile;  ///< file with commands to execute in interactive mode
    QString sslFileName; ///< Use this SSL file instead of one of the hard-coded ones.

    /// Contains all known entrypoints for the Prog.
    std::vector<Address> m_entryPoints;

    /// A vector containing the names of all symbol files to load.
    std::vector<QString> m_symbolFiles;

    /// A map to find a name by a given address.
    std::map<Address, QString> m_symbolMap;

private:
    QDir m_workingDirectory; ///< Directory where Boomerang is run from
    QDir m_dataDirectory;
    QDir m_pluginDirectory;
    QDir m_outputDirectory;
};
