#include <qapplication.h>
#include <qthread.h>
#include "mainform.h"
#include "filemonitor.h"
#include <iostream>

int main(int argc, char **argv)
{
    QApplication a(argc, argv); 
    MainForm *w = new MainForm();
    w->show();
    return a.exec();
}

