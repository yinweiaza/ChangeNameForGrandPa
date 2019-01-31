#include "udisk.h"

#include <dbt.h>
#include <QDebug>
#include <QProcess>
#include <QFileInfoList>
#include <QMessageBox>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
UDisk::UDisk(QWidget *parent) : QWidget(parent)
{

}

bool UDisk::formatUdisk(QString udiskpath)
{
    udiskpath = udiskpath.left(2);
    QString program = "cmd.exe";
    QStringList arguments;
    arguments << "/c"<< "format " + udiskpath +" /FS:FAT32 /Q /Y";
    qDebug()<< program << arguments;

    QProcess p;
    p.start(program, arguments);
    QEventLoop loop;
    while(p.state() == QProcess::Running)
    {
        QTimer::singleShot(500, &loop, SLOT(quit()));
        loop.exec();
    }
    return true;
}

QVector<udisk_list_t> UDisk::scanUdisk()
{
    QVector<udisk_list_t> udiskLists;
    QFileInfoList list =  QDir::drives();  //获取当前系统的盘符
    bool bUsb = false;
    for(int i=0;i<list.count();i++){
        qDebug()<<list[i].filePath();
        UINT ret = GetDriveType((WCHAR *) list[i].filePath().utf16());
        qDebug()<<ret;
        if(ret == DRIVE_REMOVABLE){
            qDebug()<<"存在可移动盘";
            bUsb = true;
            udisk_list_t  udiskTmp;
            udiskTmp.udisk_path = list[i].filePath();
            udiskTmp.is_check = true;
            if( GetVolumeInformation((LPCWSTR)(list[i].filePath().utf16()), 0, 0,0,0,0,0,0) )
                udiskTmp.is_ready = true;
            else
                udiskTmp.is_ready = false;
            udiskLists.append(udiskTmp);
        }
    }

    if(!bUsb){
        QMessageBox::warning(this,"warning",QStringLiteral("未检测到U盘！"),QMessageBox::Yes);
    }
    return udiskLists;
}

bool UDisk::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if(msgType == WM_DEVICECHANGE) {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags ==0) {
                    QString USBDisk = QString(this->deviceMask(lpdbv ->dbcv_unitmask)) + ":/";
                    emit sigUDiskCome(USBDisk);
                }
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags == 0) {
                    QString USBDisk = QString(this->deviceMask(lpdbv ->dbcv_unitmask)) +":/";
                    emit sigUDiskRemove(USBDisk);
                }
            }
            break;
        case DBT_DEVNODES_CHANGED:
            break;
        default:
            break;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

char UDisk::deviceMask(ulong unitmask)
{
    char i;
    for (i = 0; i < 26; ++i) {
        if( unitmask &0x1) break;
        unitmask = unitmask>>1;
    }
    return (i + 'A');
}

