/****************************************************************************
** Form implementation generated from reading ui file 'detailswidget.ui'
**
** Created: Thu Sep 4 19:50:49 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include <iostream>
#include "detailswidget.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qvbox.h>


/*
 *  Constructs a DetailsWidget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
DetailsWidget::DetailsWidget( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "DetailsWidget" );

    QVBoxLayout *vt = new QVBoxLayout(this);

    QHBoxLayout *h1 = new QHBoxLayout(vt);
    QHBoxLayout *h2 = new QHBoxLayout(vt);

    textLabel1 = new QLabel( "Name",  this );
    nameLineEdit = new QLineEdit( this, "nameLineEdit" );
    h1->addWidget(textLabel1);
    h1->addWidget(nameLineEdit);

    QVBoxLayout *v1 = new QVBoxLayout( h2 );
    QLabel *paramsLabel = new QLabel( tr( "Parameters" ), this );
    paramsListView = new QListView( this, "paramsListView" );
    paramsListView->addColumn( tr( "Name" ) );
    paramsListView->addColumn( tr( "Type" ) );
    paramsListView->addColumn( tr( "Exp" ) );
    v1->addWidget(paramsLabel);
    v1->addWidget(paramsListView);

    QVBoxLayout *v2 = new QVBoxLayout( h2 );
    QLabel *returnsLabel = new QLabel( tr( "Returns" ), this );
    returnsListView = new QListView( this, "returnsListView" );
    returnsListView->addColumn( tr( "Type" ) );
    returnsListView->addColumn( tr( "Exp" ) );
    v2->addWidget(returnsLabel);
    v2->addWidget(returnsListView);

    languageChange();
    resize( QSize(600, 482).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // buddies
    textLabel1->setBuddy( nameLineEdit );
}

/*
 *  Destroys the object and frees any allocated resources
 */
DetailsWidget::~DetailsWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void DetailsWidget::languageChange()
{
}

void DetailsWidget::clear()
{
    nameLineEdit->clear();
    paramsListView->clear();
    returnsListView->clear();
}

void DetailsWidget::setName(const QString &nam)
{
    nameLineEdit->setText(nam);
}

void DetailsWidget::addParam(const QString &nam, 
                             const QString &ty, 
                             const QString &e)
{
    QListViewItem *i = new QListViewItem(paramsListView);
    i->setText(0, nam);
    i->setText(1, ty);
    i->setText(2, e);
    paramsListView->insertItem(i);
}

void DetailsWidget::addReturn(const QString &ty, 
                              const QString &e)
{
    QListViewItem *i = new QListViewItem(returnsListView);
    i->setText(0, ty);
    i->setText(1, e);
    returnsListView->insertItem(i);
}

