#ifndef SAVETHREAD_H
#define SAVETHREAD_H

#include <QThread>
#include <QFile>
#include <QDebug>
class SaveThread : public QThread
{
    Q_OBJECT
public:
    SaveThread();

    void    setPath(QString szSrcPath, QString szSavepath){  m_srcPath = szSrcPath;  m_dstPath = szSavepath;}

protected:
    void run() override{                        //实际
        qDebug() << m_srcPath;
        if( QFile::exists(m_srcPath) )
        {
            QFile::copy(m_srcPath, m_dstPath);                                  //复制；
        }
        emit copyFinished();
    }
signals:
    void          copyFinished();
public slots:


private:
    QString         m_srcPath;
    QString         m_dstPath;
};

#endif // SAVETHREAD_H
