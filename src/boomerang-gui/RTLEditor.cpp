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


RTLEditor::RTLEditor(Decompiler *_decompiler, const QString& _name)
    : decompiler(_decompiler)
    , name(_name)
{
    updateContents();
    setMouseTracking(true);
    setReadOnly(true);
}


void RTLEditor::updateContents()
{
    QString rtl;

    decompiler->getRTLForProc(name, rtl);
    int n = verticalScrollBar()->value();
    setHtml(rtl);
    verticalScrollBar()->setValue(n);
}


void RTLEditor::mouseMoveEvent(QMouseEvent *_event)
{
    QString _name = anchorAt(_event->pos());

    if (!_name.isEmpty()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    }
    else {
        QApplication::restoreOverrideCursor();
    }
}


void RTLEditor::mousePressEvent(QMouseEvent *_event)
{
    // allow clicking on subscripts
    QString _name = anchorAt(_event->pos());

    if (!_name.isEmpty()) {
        scrollToAnchor(_name.mid(1));
        return;
    }

    QTextEdit::mousePressEvent(_event);
}
