#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QMessageBox>
#include <QFile>

#include <iostream>

using namespace std;
class TcpThread : public QThread
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = 0);

    void run();

    void connectSevere();

    void sendMessage(QString str);

    bool connectState();

signals:

private:
    QTcpSocket *socket;

    QFile musicFile;
    QFile file;         //需要接收的文件对象
    QString fileName;
    int fileSize;

    int currentSize;    //当前已接收的文件大小

    bool isHead = true;        //标志位，是否是文件头


public slots:
    void readMessage();
};

#endif // TCPTHREAD_H
