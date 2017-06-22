#include "rtleditor.h"
#include "DecompilerThread.h"

#include <QtWidgets>


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

	decompiler->getRtlForProc(name, rtl);
	int n = verticalScrollBar()->value();
	setHtml(rtl);
	verticalScrollBar()->setValue(n);
}


void RTLEditor::mouseMoveEvent(QMouseEvent *event)
{
	QString _name = anchorAt(event->pos());

	if (!_name.isEmpty()) {
		QApplication::setOverrideCursor(Qt::PointingHandCursor);
	}
	else {
		QApplication::restoreOverrideCursor();
	}
}


void RTLEditor::mousePressEvent(QMouseEvent *event)
{
	// allow clicking on subscripts
	QString _name = anchorAt(event->pos());

	if (!_name.isEmpty()) {
		scrollToAnchor(_name.mid(1));
		return;
	}

	QTextEdit::mousePressEvent(event);
}
