/****************************************************************************
** Form interface generated from reading ui file 'detailswidget.ui'
**
** Created: Thu Sep 4 19:50:37 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef DETAILSWIDGET_H
#define DETAILSWIDGET_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QLineEdit;
class QListView;
class QListViewItem;

class DetailsWidget : public QWidget
{
    Q_OBJECT

public:
    DetailsWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~DetailsWidget();

    void clear();
    void setName(const QString &nam);
    void addParam(const QString &nam, const QString &ty, const QString &e);
    void addReturn(const QString &ty, const QString &e);

protected:
    QLabel* textLabel1;
    QLineEdit* nameLineEdit;
    QListView* paramsListView;
    QListView* returnsListView;

protected slots:
    virtual void languageChange();

};

#endif // DETAILSWIDGET_H
