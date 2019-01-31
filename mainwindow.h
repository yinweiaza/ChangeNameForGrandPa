#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "udisk.h"
#include "savethread.h"
#include <QCloseEvent>
#include "udiskstatus.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void        setDisk(UDisk *pDisk);
private slots:
    void on_pushButton_choose_path_clicked();

    void on_pushButton_bat_rename_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

protected:
    void        init();
    void closeEvent(QCloseEvent *event);

    /**
     * @brief getVolumeFreeSpace        检查u盘可用内存；
     * @param szPath
     * @return
     */
    void         getVolumeFreeSpace(QString szPath, ULONGLONG  &totalByte, ULONGLONG &freeByte);
    void         updateUDiskProgress();

    void        updateDlgStatus();                  //刷新界面；
private:
    Ui::MainWindow *ui;
    UDisk               *m_pdisk;
    SaveThread     * m_pThread;
    UdiskStatus     * m_pDiskStatus;
    bool                  m_bOver;                      //关闭窗口；
};

#endif // MAINWINDOW_H
