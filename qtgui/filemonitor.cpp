
#include "filemonitor.h"
#include <iostream>
#include <qtimer.h>

FileMonitor::FileMonitor(const char *fname) : filename(fname)
{ 
    t = new QTimer(this);
    connect( t, SIGNAL(timeout()), SLOT(poll()) );
    t->start(100, FALSE);
}

void FileMonitor::poll()
{
    struct stat st;
    if (stat(getFileName(), &st) == 0 && 
        (st.st_mtime != mod_time || st.st_size != size)) {
        mod_time = st.st_mtime;
        size = st.st_size;
        emit changed();
    } 
}

