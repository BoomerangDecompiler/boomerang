#ifndef MAINFORM_H
#define MAINFORM_H

#include <qvariant.h>
#include <qmainwindow.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QSplitter;
class QListView;
class QListViewItem;
class QTabWidget;
class QWidget;
class QTextBrowser;
class QWidgetStack;
class DetailsWidget;
class QPushButton;
class FileMonitor;

class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    MainForm( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~MainForm();

    void statusMessage(const char *msg);

protected:
    QSplitter* splitter1;
    QListView* listView2;
    QWidgetStack* widgetStack;
    QTabWidget* tabWidget2;
    DetailsWidget* details;
    QTextBrowser* logger;
    QTextBrowser* decoded;
    QTextBrowser* analysed;
    QTextBrowser* ssa;
    QTextBrowser* code;
    QMenuBar *MenuBarEditor;
    QPopupMenu *FileMenu;
    QPopupMenu *EditMenu;
    QAction* fileNewAction;
    QAction* fileOpenAction;
    QAction* fileExitAction;
    QPushButton* showLog;
    QHBoxLayout* MainFormLayout;
    FileMonitor *logMonitor, *cgMonitor, 
                *detailsMonitor, *decodedMonitor, *analysedMonitor, *ssaMonitor;
    QString projectPath;

protected slots:
    virtual void languageChange();
    virtual void closeEvent(QCloseEvent *e);
    virtual void fileOpen();
    virtual void fileExit();
    virtual void listSelectionChanged(QListViewItem *item);
    virtual void showLogToggled(bool on);
    virtual void updateLog();
    virtual void updateCallGraph();
    virtual void updateDetails();
    virtual void updateDecoded();
    virtual void updateAnalysed();
    virtual void updateSSA();
};

#endif // MAINFORM_H
