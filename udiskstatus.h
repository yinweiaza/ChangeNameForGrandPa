#ifndef UDISKSTATUS_H
#define UDISKSTATUS_H

#include <QThread>
#include <QEventLoop>
#include <windows.h>
class UdiskStatus : public QThread
{
    Q_OBJECT
protected:
    void run() override{                        //实际
//        QEventLoop loop;
        while(1)
        {
            Sleep(1000);
//            QTimer::singleShot(1000, &loop, SLOT(quit()));
//            loop.exec();
            emit updateProgress();
        }
    }
signals:
    void            updateProgress();               //刷新；
public:
    UdiskStatus();    
};

#endif // UDISKSTATUS_H
