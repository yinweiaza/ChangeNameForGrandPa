#ifndef UDISK_H
#define UDISK_H

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <windows.h>
typedef struct udisk_list{
    QString udisk_path;
    bool is_ready;
    bool is_check;
}udisk_list_t;

class UDisk : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit UDisk(QWidget *parent = nullptr);
    /**
     * @brief formatUdisk   格式化U盘；
     * @param udiskpath
     * @return
     */
    bool   formatUdisk(QString  udiskpath);
    /**
     * @brief scanUdisk     扫描已经连接的U盘；
     * @return
     */
    QVector<udisk_list_t>  scanUdisk();
protected:
    /**
     * @brief nativeEventFilter     本地事件
     * @param eventType
     * @param message
     * @param result
     * @return
     */
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);


signals:
    void sigUDiskCome(QString uDiskName);
    void sigUDiskRemove(QString uDiskName);

public slots:

private:
    char  deviceMask(ulong unitmask);
};

#endif // UDISK_H
