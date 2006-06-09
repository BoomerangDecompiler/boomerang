
#ifndef RTLEDITOR_H
#define RTLEDITOR_H

#include <QtCore/QString>
#include <QtGui/QTextEdit>

#include "types.h"
#include <vector>
#include <map>
#include <set>

class Decompiler;

class RTLEditor : public QTextEdit
{
    Q_OBJECT

public:
	RTLEditor(Decompiler *decompiler, const QString &name);

public slots:
	void updateContents();

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);

private:
    Decompiler *decompiler;
	QString name;
};

#endif