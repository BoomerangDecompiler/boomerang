#include "mainform.h"
#include "detailswidget.h"
#include <iostream>
#include <fstream>

#include <qapplication.h>
#include <qvariant.h>
#include <qsplitter.h>
#include <qheader.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qstatusbar.h>
#include <qtextbrowser.h>
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qxml.h>
#include "filemonitor.h"
#include "util.h"

/*
 *  Constructs a MainForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
MainForm::MainForm( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl ), detailsMonitor(NULL),
      decodedMonitor(NULL), ssaMonitor(NULL)
{
    statusBar()->message("Ready");

    showLog = new QPushButton("show log", statusBar());
    showLog->setToggleButton(true);
    statusBar()->addWidget(showLog, 0, TRUE);

    QObject::connect( showLog, SIGNAL(toggled(bool)),
                      this,  SLOT(showLogToggled(bool)) );

    if ( !name )
	setName( "MainForm" );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
    MainFormLayout = new QHBoxLayout( centralWidget(), 2, 6, "MainFormLayout"); 

    widgetStack = new QWidgetStack( centralWidget(), "widgetStack");

    splitter1 = new QSplitter( widgetStack, "splitter1" );
    splitter1->setOrientation( QSplitter::Horizontal );
    widgetStack->addWidget(splitter1);

    listView2 = new QListView( splitter1, "listView2" );
    listView2->addColumn( "Procedures" );
    listView2->setResizeMode( QListView::LastColumn );

    QObject::connect( listView2, SIGNAL(selectionChanged(QListViewItem*)),
                      this,  SLOT(listSelectionChanged(QListViewItem*)) );

    tabWidget2 = new QTabWidget( splitter1, "tabWidget2" );

    logger = new QTextBrowser( widgetStack, "log");
    widgetStack->addWidget(logger);
    logger->setText("test");
    logger->setTextFormat(Qt::LogText);
    logger->mimeSourceFactory()->setFilePath("./");
    logger->mimeSourceFactory()->setExtensionType("", "text/plain");
    logger->setSource("../output/log");

    logMonitor = new FileMonitor(logger->source());
    QObject::connect( logMonitor, SIGNAL(changed()),
                      this,  SLOT(updateLog()) );
    cgMonitor = new FileMonitor("../output/callgraph.xml");
    QObject::connect( cgMonitor, SIGNAL(changed()),
                      this,  SLOT(updateCallGraph()) );

    widgetStack->raiseWidget(splitter1);

    details = new DetailsWidget( tabWidget2, "details" );
    tabWidget2->insertTab( details, "Details of this procedure" );

    decoded = new QTextBrowser( tabWidget2, "decoded" );
    tabWidget2->insertTab( decoded, "Display the semantics as decoded" );
    //decoded->mimeSourceFactory()->setFilePath(".");
    //decoded->setTextFormat(Qt::RichText);

    ssa = new QTextBrowser( tabWidget2, "ssa" );
    tabWidget2->insertTab( ssa, "Display the semantics in SSA form" );
    //ssa->mimeSourceFactory()->setFilePath(".");
    //ssa->setTextFormat(Qt::RichText);

    code = new QTextBrowser( tabWidget2, "code" );
    tabWidget2->insertTab( code, "Display the generated code" );
    //code->mimeSourceFactory()->setFilePath(".");

    MainFormLayout->addWidget( widgetStack );

    // actions
    fileNewAction = new QAction( this, "fileNewAction" );
    fileExitAction = new QAction( this, "fileExitAction" );
    
    QObject::connect( fileExitAction, SIGNAL(activated()),
                      this,  SLOT(fileExit()) );

    // toolbars


    // menubar
    MenuBarEditor = new QMenuBar( this, "MenuBarEditor" );


    FileMenu = new QPopupMenu( this );
    fileNewAction->addTo( FileMenu );
    FileMenu->insertSeparator();
    fileExitAction->addTo( FileMenu );
    MenuBarEditor->insertItem( "", FileMenu, 1 );

    EditMenu = new QPopupMenu( this );
    MenuBarEditor->insertItem( "", EditMenu, 2 );


    languageChange();
    resize( QSize(656, 400).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
MainForm::~MainForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MainForm::languageChange()
{
    setCaption( tr( "Boomerang" ) );
    listView2->header()->setLabel( 0, "Procedures" );
    listView2->clear();
/*    QListViewItem * item_2 = new QListViewItem( listView2, 0 );
    item_2->setOpen( TRUE );
    QListViewItem * item = new QListViewItem( item_2, 0 );
    item->setText( 0, tr( "fib" ) );
    item_2->setText( 0, tr( "main" ) );
    listView2->setSelected(item_2, true);
*/

    tabWidget2->changeTab( details, tr( "Details" ) );
    tabWidget2->changeTab( decoded, tr( "Decoded" ) );
    tabWidget2->changeTab( ssa, tr( "SSA" ) );
    tabWidget2->changeTab( code, tr( "Code" ) );
    fileNewAction->setText( tr( "New" ) );
    fileExitAction->setText( tr( "Exit" ) );
    MenuBarEditor->findItem( 1 )->setText( tr( "File" ) );
    MenuBarEditor->findItem( 2 )->setText( tr( "Edit" ) );
}

void MainForm::closeEvent( QCloseEvent * )
{
    fileExit();
}

void MainForm::fileExit()
{
    logMonitor->stop();
    QApplication::exit( 0 );
}

void MainForm::listSelectionChanged(QListViewItem *item)
{
    updateDetails();
    updateDecoded();
    updateSSA();
}

void MainForm::showLogToggled(bool on)
{
    if (on) {
        widgetStack->raiseWidget(logger);
        updateLog();
    } else
        widgetStack->raiseWidget(splitter1);
}

void MainForm::statusMessage(const char *msg)
{
    statusBar()->message(msg);
}

void MainForm::updateLog()
{
    if (widgetStack->visibleWidget() != logger)
        return;
    QScrollBar *v = logger->verticalScrollBar();
    int value = v->value();
    bool max = (value > v->maxValue() - 10);
    /* at some point in time it may be necessary to use the 
       QFile class and manually update the log (for efficiency).
       But at the moment we'll just reload it
     */
    logger->reload();
    if (max)
        logger->scrollToBottom();
    else
        logger->verticalScrollBar()->setValue(value);
}

class CGHandler : public QXmlDefaultHandler
{
public:
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& , 
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );

    QListView *listView2;

protected:
    std::list<QListViewItem*> itemStack;
};                   

void MainForm::updateCallGraph()
{
    QString fname = "../output/callgraph.xml";
    int fd = lockFileRead(fname);
    QFile f(fname);
    if (f.open(IO_ReadOnly) == FALSE) {
        std::cerr << "cannot open " << fname << std::endl;
        return;
    }
    QXmlSimpleReader q;
    QXmlInputSource i(f);
    CGHandler h;
    h.listView2 = listView2;
    q.setContentHandler(&h);
    q.parse(i);
    unlockFile(fd);
}

bool CGHandler::startDocument()
{
    listView2->clear();
    itemStack.clear();
    return TRUE;
}

bool CGHandler::startElement( const QString&, const QString&, 
                                    const QString& qName, 
                                    const QXmlAttributes &a )
{
    if (qName != "proc")
        return TRUE;
    QListViewItem *item;
    if (itemStack.size() == 0)
        item = new QListViewItem( listView2, 0 );
    else
        item = new QListViewItem( itemStack.front(), 0 );
    item->setText( 0, a.value("name") );
    if (itemStack.size() == 0) {
        item->setOpen( TRUE );
        listView2->setSelected(item, true);
    }
    itemStack.push_front(item);
    return TRUE;
}

bool CGHandler::endElement( const QString&, const QString&, const QString& qName )
{
    if (qName != "proc")
        return TRUE;
    if (itemStack.size() == 0)
        std::cerr << "no tags in stack!\n";
    else
        itemStack.erase(itemStack.begin());
    return TRUE;
}

class DetailsHandler : public QXmlDefaultHandler
{
public:
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& , 
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );

    DetailsWidget *details;
    
protected:
};                   

void MainForm::updateDetails()
{
    QString fname = QString("../output/") + 
                    listView2->selectedItem()->text(0) + 
                    "-details.xml";
    int fd = lockFileRead(fname);
    QFile f(fname);
    if (f.open(IO_ReadOnly) == FALSE) {
        std::cerr << "cannot open " << fname << std::endl;
        return;
    }
    QXmlSimpleReader q;
    QXmlInputSource i(f);
    DetailsHandler h;
    h.details = details;
    q.setContentHandler(&h);
    q.parse(i);
    unlockFile(fd);

    if (detailsMonitor)
        detailsMonitor->setFileName(fname);
    else
        detailsMonitor = new FileMonitor(fname);
}

bool DetailsHandler::startDocument()
{
    details->clear();
    return TRUE;
}

bool DetailsHandler::startElement( const QString&, const QString&, 
                                    const QString& qName, 
                                    const QXmlAttributes &a )
{
    if (qName == "proc") {
        details->setName(a.value("name"));
        return TRUE;
    }
    if (qName == "param") {
        details->addParam(a.value("name"), a.value("type"), a.value("exp"));
        return TRUE;
    }
    if (qName == "return") {
        details->addReturn(a.value("type"), a.value("exp"));
        return TRUE;
    }
    return TRUE;
}

bool DetailsHandler::endElement( const QString&, const QString&, const QString& qName )
{
    return TRUE;
}

class DecodedHandler : public QXmlDefaultHandler
{
public:
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& , 
                       const QXmlAttributes& );
    bool characters( const QString& ch );
    bool endElement( const QString&, const QString&, const QString& );

    QTextBrowser *decoded;
    
protected:
    bool startDecoded;
};                   

void MainForm::updateDecoded()
{
    QString fname = QString("../output/") + 
                    listView2->selectedItem()->text(0) + 
                    "-decoded.xml";
    int fd = lockFileRead(fname);
    QFile f(fname);
    if (f.open(IO_ReadOnly) == FALSE) {
        std::cerr << "cannot open " << fname << std::endl;
        return;
    }
    QXmlSimpleReader q;
    QXmlInputSource i(f);
    DecodedHandler h;
    h.decoded = decoded;
    q.setContentHandler(&h);
    q.parse(i);
    unlockFile(fd);

    if (decodedMonitor)
        decodedMonitor->setFileName(fname);
    else
        decodedMonitor = new FileMonitor(fname);
}

bool DecodedHandler::startDocument()
{
    decoded->clear();
    startDecoded = false;
    return TRUE;
}

bool DecodedHandler::startElement( const QString&, const QString&, 
                                    const QString& qName, 
                                    const QXmlAttributes &a )
{
    if (qName == "proc") {
        return TRUE;
    }
    if (qName == "decoded") {
        startDecoded = true;
        return TRUE;
    }
    return TRUE;
}

bool DecodedHandler::endElement( const QString&, const QString&, const QString& qName )
{
    if (qName == "proc") {
        return TRUE;
    }
    if (qName == "decoded") {
        startDecoded = false;
        return TRUE;
    }
    return TRUE;
}

bool DecodedHandler::characters( const QString& ch )
{
    if (!startDecoded) return TRUE;
    decoded->setText(ch);
    return TRUE;
}

class SSAHandler : public QXmlDefaultHandler
{
public:
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& , 
                       const QXmlAttributes& );
    bool characters( const QString& ch );
    bool endElement( const QString&, const QString&, const QString& );

    QTextBrowser *ssa;
    
protected:
    bool startSSA;
};                   

void MainForm::updateSSA()
{
    QString fname = QString("../output/") + 
                    listView2->selectedItem()->text(0) + 
                    "-ssa.xml";
    int fd = lockFileRead(fname);
    QFile f(fname);
    if (f.open(IO_ReadOnly) == FALSE) {
        std::cerr << "cannot open " << fname << std::endl;
        return;
    }
    QXmlSimpleReader q;
    QXmlInputSource i(f);
    SSAHandler h;
    h.ssa = ssa;
    q.setContentHandler(&h);
    q.parse(i);
    unlockFile(fd);

    if (ssaMonitor)
        ssaMonitor->setFileName(fname);
    else
        ssaMonitor = new FileMonitor(fname);
}

bool SSAHandler::startDocument()
{
    ssa->clear();
    startSSA = false;
    return TRUE;
}

bool SSAHandler::startElement( const QString&, const QString&, 
                                    const QString& qName, 
                                    const QXmlAttributes &a )
{
    if (qName == "proc") {
        return TRUE;
    }
    if (qName == "ssa") {
        startSSA = true;
        return TRUE;
    }
    return TRUE;
}

bool SSAHandler::endElement( const QString&, const QString&, const QString& qName )
{
    if (qName == "proc") {
        return TRUE;
    }
    if (qName == "ssa") {
        startSSA = false;
        return TRUE;
    }
    return TRUE;
}

bool SSAHandler::characters( const QString& ch )
{
    if (!startSSA) return TRUE;
    ssa->setText(ch);
    return TRUE;
}

