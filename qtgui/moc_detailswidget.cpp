/****************************************************************************
** DetailsWidget meta object code from reading C++ file 'detailswidget.h'
**
** Created: Fri Sep 5 09:21:35 2003
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "detailswidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *DetailsWidget::className() const
{
    return "DetailsWidget";
}

QMetaObject *DetailsWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_DetailsWidget( "DetailsWidget", &DetailsWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString DetailsWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DetailsWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString DetailsWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DetailsWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* DetailsWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "languageChange()", &slot_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"DetailsWidget", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_DetailsWidget.setMetaObject( metaObj );
    return metaObj;
}

void* DetailsWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "DetailsWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool DetailsWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: languageChange(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool DetailsWidget::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool DetailsWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool DetailsWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
