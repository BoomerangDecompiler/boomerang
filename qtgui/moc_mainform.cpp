/****************************************************************************
** MainForm meta object code from reading C++ file 'mainform.h'
**
** Created: Mon Sep 15 13:22:49 2003
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "mainform.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MainForm::className() const
{
    return "MainForm";
}

QMetaObject *MainForm::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MainForm( "MainForm", &MainForm::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MainForm::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MainForm", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MainForm::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MainForm", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MainForm::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"languageChange", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUMethod slot_2 = {"fileExit", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ "item", &static_QUType_ptr, "QListViewItem", QUParameter::In }
    };
    static const QUMethod slot_3 = {"listSelectionChanged", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"showLogToggled", 1, param_slot_4 };
    static const QUMethod slot_5 = {"updateLog", 0, 0 };
    static const QUMethod slot_6 = {"updateCallGraph", 0, 0 };
    static const QUMethod slot_7 = {"updateDetails", 0, 0 };
    static const QUMethod slot_8 = {"updateDecoded", 0, 0 };
    static const QUMethod slot_9 = {"updateSSA", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "languageChange()", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "fileExit()", &slot_2, QMetaData::Protected },
	{ "listSelectionChanged(QListViewItem*)", &slot_3, QMetaData::Protected },
	{ "showLogToggled(bool)", &slot_4, QMetaData::Protected },
	{ "updateLog()", &slot_5, QMetaData::Protected },
	{ "updateCallGraph()", &slot_6, QMetaData::Protected },
	{ "updateDetails()", &slot_7, QMetaData::Protected },
	{ "updateDecoded()", &slot_8, QMetaData::Protected },
	{ "updateSSA()", &slot_9, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"MainForm", parentObject,
	slot_tbl, 10,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MainForm.setMetaObject( metaObj );
    return metaObj;
}

void* MainForm::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MainForm" ) )
	return this;
    return QMainWindow::qt_cast( clname );
}

bool MainForm::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: languageChange(); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: fileExit(); break;
    case 3: listSelectionChanged((QListViewItem*)static_QUType_ptr.get(_o+1)); break;
    case 4: showLogToggled((bool)static_QUType_bool.get(_o+1)); break;
    case 5: updateLog(); break;
    case 6: updateCallGraph(); break;
    case 7: updateDetails(); break;
    case 8: updateDecoded(); break;
    case 9: updateSSA(); break;
    default:
	return QMainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MainForm::qt_emit( int _id, QUObject* _o )
{
    return QMainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MainForm::qt_property( int id, int f, QVariant* v)
{
    return QMainWindow::qt_property( id, f, v);
}

bool MainForm::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
