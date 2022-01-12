#include "tcpthread.h"

TcpThread::TcpThread(QObject *parent) : QThread(parent)
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &TcpThread::readMessage);          //接收信息
    this->connectSevere();
}


void TcpThread::run()
{

}


bool TcpThread::connectState()
{
    if(socket->ConnectedState == QAbstractSocket::UnconnectedState)
    {
        return false;
    }
    else
        return true;
}

//连接服务器
void TcpThread::connectSevere()
{
    QString ipstr = "127.0.0.1";
    quint16 currentPort = 8888;
    socket->connectToHost(ipstr, currentPort);
    if(socket->waitForConnected())
    {
        qDebug() << "连接成功" << endl;
    }
    else
    {
        qDebug() << "连接失败" << endl;
    }
}


//接收服务器信息
void TcpThread::readMessage()    //接收信息
{
    QByteArray content = socket->readAll();
//    QString str;
//    str = arr.data();
//    QMessageBox::information(NULL, "洲哥的信息", str, QMessageBox::Yes);
    if(true == isHead)
    {
        isHead = false;
        fileName = QString(content).section("###", 0, 0);
        fileSize = QString(content).section("###", 1, 1).toInt();

        cout << "fileName is " << fileName.toStdString() << endl;
        //重置已接收大小
        currentSize = 0;
        file.setFileName(fileName);
        //以只写方式打开文件
        if(!file.open(QIODevice::WriteOnly))
        {
            qDebug()<<"以只写方式打开文件失败";
            return ;
        }
    }
    else
    {
        cout << "write file content" << endl;

        qint64 len = file.write(content);
        //每次写入的大小累加
        if(len > 0)
        {
            currentSize += len;
        }

        if(currentSize == fileSize)   //文件传输完成的条件
        {
            QMessageBox::information(NULL, "完成", "文件传输完成");

            //传输完成,关闭文件且关闭连接
            file.close();
        }

    }

    isHead = true;

//    musicFile.open(QIODevice::WriteOnly);
//    musicFile.setFileName("./music.mp3");
//    musicFile.write(arr);
//    musicFile.close();
}


//发送信息
void TcpThread::sendMessage(QString str)
{
    if(socket->isOpen() && socket->isValid())
    {
        socket->write(str.toUtf8());    //给服务端发送信息
    }

}






