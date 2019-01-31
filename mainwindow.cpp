#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfoList>
#include <QThread>
#include "savethread.h"
#include <windows.h>
#include <QList>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QTime>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pThread(new SaveThread),
    m_bOver(false),
    m_pDiskStatus(new UdiskStatus)
{
    ui->setupUi(this);
    this->init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setDisk(UDisk *pDisk)
{
    m_pdisk = pDisk;
    QObject::connect(m_pdisk, &UDisk::sigUDiskCome, [=](QString uDiskName){
        bool bFind = false;
        for(int i = 0; i< ui->comboBox->count(); i++ )
        {
            if(ui->comboBox->itemText(i).compare(uDiskName) == 0 )
                bFind = true;
        }
        if( !bFind )
            ui->comboBox->addItem(uDiskName);
    });
    QObject::connect(m_pdisk, &UDisk::sigUDiskRemove,[=](QString uDiskName) {
        int index = -1;
        for(int i = 0; i< ui->comboBox->count(); i++ )
        {
            if( ui->comboBox->itemText(i).compare(uDiskName) == 0)
                index = i;
        }
        if( -1!= index)
            ui->comboBox->removeItem(index);
    });
}

void MainWindow::on_pushButton_choose_path_clicked()
{
    QFileDialog fileDlg(this);
    fileDlg.setWindowTitle("请选择戏曲存放的文件夹: ");
    fileDlg.setFileMode(QFileDialog::DirectoryOnly);
    if( fileDlg.exec() == QDialog::Accepted)
    {
        QStringList szSelect = fileDlg.selectedFiles();
        ui->lineEdit_input_path->setText(szSelect[0]);
    }
}

/**
 * @brief MainWindow::on_pushButton_bat_rename_clicked     先检查，  格式化U盘，  再开始存；
 */
void MainWindow::on_pushButton_bat_rename_clicked()
{
    QTextEdit *pInfo = ui->textEdit;
    if( ui->lineEdit_input_path->text().trimmed().isEmpty() )
    {
        QMessageBox::warning(NULL, "警告", "请指定戏曲路径!!",QMessageBox::Yes);
        return;
    }
    QString szPath= ui->lineEdit_input_path->text().trimmed();
    if( !QFile::exists(szPath))
    {
        QMessageBox::warning(NULL, "警告","路径不存在！！", QMessageBox::Yes);
        return;
    }
    if( ui->comboBox->currentText().trimmed().isEmpty())
    {
        QMessageBox::warning(NULL, "警告", "不存在U盘", QMessageBox::Yes);
        return;
    }
    QString szUDisk = ui->comboBox->currentText().trimmed();
    if( ui->checkBox->isChecked() ){
        pInfo->append("即将格式化U盘.....");
        QMessageBox dlg(QMessageBox::NoIcon, "格式化?", "即将格式化U盘。。。", QMessageBox::Yes | QMessageBox::No, NULL);
        if( dlg.exec() == QMessageBox::Yes)
        {
            pInfo->append("正在格式化U盘");
            m_pdisk->formatUdisk(szUDisk);                              //格式化U盘；
            pInfo->append("格式化U盘完成");
        }
    }

    m_pDiskStatus->start();                                 //格式化后才能开启状态读取；

    pInfo->append("开始复制戏曲到U盘");
    QDir dir(ui->lineEdit_input_path->text().trimmed());
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::AllDirs);
    dir.setSorting(QDir::Name);
    QFileInfoList  fileList = dir.entryInfoList();
    QFileInfoList::const_iterator iter = fileList.begin();
    pInfo->append(QString("戏曲：%1").arg(dir.count()-2));         //除掉., ..
    int index = 1;
    connect(m_pThread, &SaveThread::copyFinished, [&](){
         pInfo->append("复制完成！");
    });

    QVector<QString>  fileVec;

    while (iter != fileList.end()) {
        updateDlgStatus();
        while( m_pThread->isRunning())
        {
            QTime delayTime = QTime::currentTime().addMSecs(1000);	//在当前时间上增加3S/* lyh delete -> 避免有拉拽小票冲突 */
            while( QTime::currentTime() < delayTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        if( m_bOver )
            break;

        QString szNameTemp = (*iter).fileName();
        QString szBackTemp = szPath + "/" + szNameTemp;
        QString szExt = szNameTemp.right(szNameTemp.size() -  szNameTemp.lastIndexOf('.')-1);
//        qDebug()<<szExt << szNameTemp << szNameTemp.size() - szNameTemp.lastIndexOf('.');
        //格式过滤；
        if( !(szExt.compare("mp4") == 0 && ui->checkBox_mp4->isChecked()) ){
            if( !(szExt.compare("flv")==0 && ui->checkBox_flv->isChecked())){
                if( !(szExt.compare("mp3") == 0 && ui->checkBox_mp3->isChecked() )){
                    if( !(szExt.compare("wav")==0 && ui->checkBox_wav->isChecked())) {
                        iter++;
                        continue;
                    }
                }
            }
        }

        //开始复制；
        pInfo->append(QString("<<开始复制:%1").arg(szNameTemp));
        // 修改名字；
        int removeIndex = -1;
        qDebug()<<szNameTemp;
        for(int i = 0; i< szNameTemp.length(); i++ )
        {
            QChar tmpchar = szNameTemp.at(i);
            if( (tmpchar >= '0' && tmpchar <='9') || tmpchar == ' ' || tmpchar == '.' || tmpchar =='、')
                removeIndex = i;
            else
                break;
        }
        if( removeIndex >= 0)
            szNameTemp = szNameTemp.right( szNameTemp.length() - removeIndex -1);
        fileVec.append(QString("%1、%2\r\n").arg(index).arg(szNameTemp));                 //新名字；
        szNameTemp =  QString("%1/%2、%3").arg(szUDisk).arg(index).arg(szNameTemp);

        pInfo->append(QString("新名字：%1").arg(szNameTemp));

        // 检查U盘可用内存与要复制的文件大小；
        QFileInfo file(szBackTemp);
        ULONGLONG  fileSize =  (ULONGLONG)file.size();                        //文件大小  字节；
        ULONGLONG  freespace , totalByte;
        freespace = totalByte = 0;
        getVolumeFreeSpace(szUDisk, totalByte, freespace );
        if( totalByte <= 0 ) return;
        ui->progressBar->setValue((totalByte - freespace)/totalByte * 1000);
        if( freespace < fileSize )
        {
            pInfo->append(QString("还剩下%1 字节!!").arg(freespace));
            continue;
        }

        //开线程及复制；
        m_pThread->setPath(szBackTemp,  szNameTemp);
        m_pThread->start();
        index++;
        iter++;
    }
    updateDlgStatus();
    if(m_bOver)
        qApp->quit();
    pInfo->append("所有的戏曲复制完成!!!");

    // 保存列表；
    QFile file(QString("%1fileList.txt").arg(szUDisk));
    if(file.open(QIODevice::ReadWrite))
    {
        for(int i =  0; i< fileVec.size(); i++ )
        {
            file.write(fileVec[i].toStdString().c_str());
        }
    }
    file.close();
    QMessageBox::information(NULL, "恭喜", "所有的戏曲已经复制完成，可以直接给爷爷了!");
}


void MainWindow::init()
{
    //检测已经连接了的U盘；
    ui->comboBox->clear();
    QVector<udisk_list_t> udiskVec =  m_pdisk->scanUdisk();
    for(int i = 0; i< udiskVec.size(); i++ )
    {
        udisk_list_t & tmpDisk = udiskVec[i];
        if( tmpDisk.is_ready )
            ui->comboBox->addItem(tmpDisk.udisk_path);
    }
//    ui->lineEdit_input_path->setText("D:/music");

    connect(m_pDiskStatus, &UdiskStatus::updateProgress, this, &MainWindow::updateUDiskProgress);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if( !m_pThread->isRunning() )               // 没有线程运行， 直接关闭；
    {
        if( m_pDiskStatus->isRunning() )
            m_pDiskStatus->exit(0);
        qApp->quit();
    }
    QMessageBox  dlg(QMessageBox::NoIcon, "要关闭？", "还有线程在运行，如果要关闭，请选择，将在当前戏曲复制完成后关闭!!",QMessageBox::Yes | QMessageBox::No, this);
    if( dlg.exec() ==  QMessageBox::Yes )
        m_bOver = true;
    else
        m_bOver = false;
    event->ignore();
}

void MainWindow::getVolumeFreeSpace(QString szPath, ULONGLONG  &totalByte, ULONGLONG &freeByte)
{
    szPath = szPath.left(2);
    qDebug()<<szPath;
    std::wstring  wpath = szPath.toStdWString();
    if( wpath.empty() )
        return ;
    LPCWSTR  lPath = (LPCWSTR)wpath.c_str();
    ULARGE_INTEGER uFreeByteAvailable, uTotalByte, uTotalFreeBytes;
    if (GetDiskFreeSpaceEx(lPath,&uFreeByteAvailable,&uTotalByte,&uTotalFreeBytes))
    {
        totalByte = uTotalByte.QuadPart;
        freeByte = uFreeByteAvailable.QuadPart;
    }
}

void MainWindow::updateUDiskProgress()
{
    QString szUDisk = ui->comboBox->currentText().trimmed();
    if( szUDisk.isEmpty()) return;
    ULONGLONG  freespace,totalByte;
    freespace = totalByte = -1;
    getVolumeFreeSpace(szUDisk,totalByte,  freespace);
    ui->progressBar->setValue(1000 *  (totalByte - freespace)/ totalByte);
}

void MainWindow::updateDlgStatus()
{
    bool bRunning = m_pThread->isRunning();

    ui->comboBox->setEnabled(!bRunning);
    ui->checkBox->setEnabled(!bRunning);
    ui->lineEdit_input_path->setEnabled(!bRunning);
    ui->pushButton_bat_rename->setEnabled(!bRunning);
    ui->pushButton_choose_path->setEnabled(!bRunning);
}

/**
 * @brief MainWindow::on_comboBox_currentIndexChanged                   改变U盘， 显示剩余内存百分比；
 * @param arg1
 */
void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
    ULONGLONG  freespace, totalbytes;
    getVolumeFreeSpace(arg1, totalbytes, freespace);
    if( totalbytes <=0 ) return;
    ui->progressBar->setValue(1000 * (totalbytes - freespace)/totalbytes);
    m_pDiskStatus->exit(0);
}
