
#include <QtGui>

#include "rtleditor.h"
#include "DecompilerThread.h"

RTLEditor::RTLEditor(Decompiler *decompiler, const QString &name) : 
    decompiler(decompiler),
	name(name)
{
	updateContents();
	setMouseTracking(true);
	setReadOnly(true);
}

void RTLEditor::updateContents()
{
	QString rtl;
	decompiler->getRtlForProc(name, rtl);
	int n = verticalScrollBar()->value();
	setHtml(rtl);
	verticalScrollBar()->setValue(n);
}

void RTLEditor::mouseMoveEvent(QMouseEvent *event)
{
	QString name = anchorAt(event->pos());
	if (!name.isEmpty())
		QApplication::setOverrideCursor(Qt::PointingHandCursor);
	else
		QApplication::restoreOverrideCursor();
}

void RTLEditor::mousePressEvent(QMouseEvent *event)
{
    // allow clicking on subscripts
	QString name = anchorAt(event->pos());
	if (!name.isEmpty()) {
		scrollToAnchor(name.mid(1));
		return;
	}
	QTextEdit::mousePressEvent(event);
}