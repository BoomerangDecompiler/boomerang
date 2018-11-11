#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RTLEditor.h"

#include "Decompiler.h"

#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>


RTLEditor::RTLEditor(Decompiler *_decompiler, const QString &_name)
    : decompiler(_decompiler)
    , name(_name)
{
    setMouseTracking(true);
    setReadOnly(true);

    updateContents();
}


void RTLEditor::updateContents()
{
    QString rtl;

    if (decompiler->getRTLForProc(name, rtl)) {
        int n = verticalScrollBar()->value();
        setPlainText(rtl);
        verticalScrollBar()->setValue(n);
    }
}
