/****************************************************************************
** FileMonitor meta object code from reading C++ file 'filemonitor.h'
**
** Created: Mon Sep 15 13:22:59 2003
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "filemonitor.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *FileMonitor::className() const
{
    return "FileMonitor";
}

QMetaObject *FileMonitor::metaObj = 0;
static QMetaObjectCleanUp cleanUp_FileMonitor( "FileMonitor", &FileMonitor::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString FileMonitor::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "FileMonitor", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString FileMonitor::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "FileMonitor", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* FileMonitor::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUMethod slot_0 = {"poll", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "poll()", &slot_0, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"changed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "changed()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"FileMonitor", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_FileMonitor.setMetaObject( metaObj );
    return metaObj;
}

void* FileMonitor::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "FileMonitor" ) )
	return this;
    return QObject::qt_cast( clname );
}

// SIGNAL changed
void FileMonitor::changed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool FileMonitor::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: poll(); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool FileMonitor::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: changed(); break;
    default:
	return QObject::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool FileMonitor::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool FileMonitor::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
