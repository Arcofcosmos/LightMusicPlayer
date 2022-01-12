#include "lyricwidget.h"
#include "ui_lyricwidget.h"

#include <QFile>
#include <QRegExp>
#include <algorithm>
#include <QDebug>
#include <QTextCodec>


LyricWidget::LyricWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LyricWidget)
{
    ui->setupUi(this);
    this->clearLyric();
}


//重载<比较歌词的先后顺序
bool operator<(const LyricLine &A, const LyricLine &B)
{
    return A.currentTime < B.currentTime;
}


//剖析歌词文件并将其存入vector中
bool LyricWidget::processLyric(QString lyricPath)
{
    //qDebug() << "歌词：" << lyricPath << endl;
    QFile lyricFile(lyricPath);
    lyricFile.open(QFile::ReadOnly);
    QString content(byteArrayToUnicode(lyricFile.readAll()));
    //qDebug() << content << endl;

    AllLyric.clear();      //先清空再存入

    QRegExp regExpMode("\\[(\\d+):(\\d+(\\.\\d+)?)\\]");           //用于查找每行中时间的正则表达式

    int pos = regExpMode.indexIn(content);       //开始匹配返回首次匹配到的字符索引,若匹配到了只匹配一次

    if(-1 == pos)            //未匹配到
    {
        return false;
    }
    else
    {
        int timeLabel;       //歌词对应时间
        int lastPos;

        do
        {
            //cap(1)为匹配到的分钟，cap(2)为匹配到的秒，是浮点型,最后转换为毫秒
            timeLabel = (regExpMode.cap(1).toInt() * 60 + regExpMode.cap(2).toDouble())*1000;
            //matchedLength返回匹配到的整个字符串长度也就是整个时间长度
            lastPos = pos + regExpMode.matchedLength();     //lastPos应为时间后的字符索引
            pos = regExpMode.indexIn(content, lastPos);   //从此处再开始匹配返回的pos应该为第一句歌词后的那个时间字符索引
            if(-1 == pos)         //若最终匹配完了，就会匹配失败
            {
                //此时lastPos为最后一行的歌词首位索引，应还有行歌词未保存
                //mid返回从arg1开始后面的歌词,arg2为返回的字符串长度，默认为全部
                //trimmed删除字符串两端的空白符
                QString text = content.mid(lastPos).trimmed();
                AllLyric.push_back(LyricLine(timeLabel, text));
                //qDebug() << text << endl;
                break;
            }

            //pos - lastPos为前一个时间点和后一个时间点之间歌词的长度
            QString text = content.mid(lastPos, pos - lastPos).trimmed();
            AllLyric.push_back(LyricLine(timeLabel, text));
            //qDebug() << text << endl;
        }while(true);

        //stable_sort是稳定排序算法，对相同的单位不会改变其相对位置
        std::stable_sort(AllLyric.begin(), AllLyric.end());         //前面已经重载"<"现根据歌词时间点对歌词进行排序

    }

    if(!AllLyric.isEmpty())
        return true;

    return false;
}


//避免出现中文乱码
QString LyricWidget::byteArrayToUnicode(const QByteArray &array)
{
    // state用于保存转换状态，它的成员invalidChars，可用来判断是否转换成功
    // 如果转换成功，则值为0，如果值大于0，则说明转换失败
    QTextCodec::ConverterState state;
    // 先尝试使用utf-8的方式把QByteArray转换成QString
    QString text = QTextCodec::codecForName("UTF-8")->toUnicode(array.constData(), array.size(), &state);
    // 如果转换时无效字符数量大于0，说明编码格式不对
    if (state.invalidChars > 0)
    {
        // 再尝试使用GBK的方式进行转换，一般就能转换正确(当然也可能是其它格式，但比较少见了)
        text = QTextCodec::codecForName("GBK")->toUnicode(array);
    }
    return text;
}



//获取当前时间点歌词对应索引
int LyricWidget::getIndex(int position)
{
    if(AllLyric.empty())
        return -1;
    else
    {
        if(AllLyric[0].currentTime > position)
            return 0;
    }

    for(int i = 1; i < AllLyric.size(); i++)
    {
        if(AllLyric[i - 1].currentTime < position && AllLyric[i].currentTime >= position)
            return i - 1;
    }

    return AllLyric.size() - 1;

}


//获取对应索引的歌词
QString LyricWidget::getIndexLyric(int index)
{
    if(index < 0 || index >= AllLyric.size())
        return "";

    else
    {
        return AllLyric[index].lyricText;
    }
}


//将当前歌词显示到界面上
void LyricWidget::showLyricToWidget(int position)
{
    int index = getIndex(position);
    if(-1 == index)            //歌词为空
    {
        ui->label_1->setText("");
        ui->label_2->setText("");
        ui->label_3->setText("");
        ui->center_label->setText(u8"当前歌曲暂无单词");
        ui->label_5->setText("");
        ui->label_6->setText("");
        ui->label_7->setText("");
    }

    else
    {
        ui->label_1->setText(getIndexLyric(index - 3));
        ui->label_2->setText(getIndexLyric(index - 2));
        ui->label_3->setText(getIndexLyric(index - 1));
        ui->center_label->setText(getIndexLyric(index));
        ui->label_5->setText(getIndexLyric(index + 1));
        ui->label_6->setText(getIndexLyric(index + 2));
        ui->label_7->setText(getIndexLyric(index + 3));
    }
}


//清空界面的歌词
void LyricWidget::clearLyric()
{
    ui->label_1->setText("");
    ui->label_2->setText("");
    ui->label_3->setText("");
    ui->center_label->setText("");
    ui->label_5->setText("");
    ui->label_6->setText("");
    ui->label_7->setText("");
}


LyricWidget::~LyricWidget()
{
    delete ui;
}
