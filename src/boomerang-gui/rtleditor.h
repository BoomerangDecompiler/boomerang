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

#include <QtCore/QString>
#include <QtWidgets/QTextEdit>


#include <vector>
#include <map>
#include <set>

class Decompiler;

class RTLEditor : public QTextEdit
{
    Q_OBJECT

public:
    RTLEditor(Decompiler *decompiler, const QString& name);

public slots:
    void updateContents();

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    Decompiler *decompiler;
    QString name;
};
