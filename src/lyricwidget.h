#ifndef LYRICWIDGET_H
#define LYRICWIDGET_H

#include <QWidget>
#include <QString>
#include <QVector>


//一行歌词类
class LyricLine
{

public:
    int     currentTime;        //当前行歌词所在时间点
    QString     lyricText;      //当前行歌词内容
    LyricLine(){}
    LyricLine(int time, QString text):currentTime(time), lyricText(text){}
};


bool operator<(const LyricLine &A, const LyricLine &B);


namespace Ui {
class LyricWidget;
}


class LyricWidget : public QWidget
{
    Q_OBJECT

private:
    QVector<LyricLine> AllLyric;          //存储一首歌曲的所有歌词


public:
    explicit LyricWidget(QWidget *parent = nullptr);

    bool    processLyric(QString lyricPath);        //剖析歌词文件将歌词存入vector中
    int     getIndex(int position);            //根据时间点获取对应的歌词索引
    void    showLyricToWidget(int position);        //将对应时间点上的歌词呈现在界面上
    QString     getIndexLyric(int index);            //获取对应索引的歌词
    void    clearLyric();                   //清空界面上的歌词
    QString byteArrayToUnicode(const QByteArray &array);       //避免出现中文乱码

    ~LyricWidget();

private:
    Ui::LyricWidget *ui;
};

#endif // LYRICWIDGET_H
