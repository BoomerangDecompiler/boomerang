
#ifndef FILEMONITOR_H
#define FILEMONITOR_H
#include <qobject.h>
#include <qtimer.h>
#include <map>
#include <sys/stat.h>
#include <string>

class FileMonitor : public QObject {
    Q_OBJECT

public:
    FileMonitor(const char *fname);
    const char *getFileName() { return filename.c_str(); }
    void stop() { t->stop(); }
    
signals:
    void changed();
    
protected slots:
    virtual void poll();

protected:
    std::string filename;
    time_t mod_time;
    off_t size;
    QTimer *t;

};

#endif
