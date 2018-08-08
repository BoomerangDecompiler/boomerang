#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SeparateLogger.h"


#include "boomerang/util/log/FileLogSink.h"
#include "boomerang/util/Util.h"

#include <QDir>
#include <QMap>
#include <QSharedPointer>


SeparateLogger::SeparateLogger(const QString& fullFilePath)
{
    QDir().remove(fullFilePath); // overwrite old logs
    addLogSink(Util::makeUnique<FileLogSink>(fullFilePath, true));
}


SeparateLogger& SeparateLogger::getOrCreateLog(const QString& name)
{
    static QMap<QString, QSharedPointer<SeparateLogger>> loggers;

    if (!loggers.contains(name)) {
        loggers[name].reset(new SeparateLogger(name + ".log"));
    }

    return *loggers[name];
}
