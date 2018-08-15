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


#include "boomerang/util/Types.h"

#include <QString>
#include <QPlainTextEdit>

#include <map>
#include <set>
#include <vector>


class Decompiler;


class RTLEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    RTLEditor(Decompiler *decompiler, const QString& name);

public slots:
    void updateContents();

private:
    Decompiler *decompiler;
    QString name;
};
