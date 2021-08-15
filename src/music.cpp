#include "music.h"
#include <QCoreApplication>
#include <QtSql>
#include <QMessageBox>
#include <QApplication>


//将毫秒的时间格式转换成标准时间QString
extern QString convertTimeFormat(int timeMilliSeconds);



Music::Music(QUrl myUrl)
{
    url = myUrl;
    this->analysisMusic();
}


//解析歌曲信息
void Music::analysisMusic()
{
    QMediaPlayer mediaPlayer;
    mediaPlayer.setMedia(url);
    //qDebug() << "url: " << url << endl;

    //解析元数据，需要一定时间
    while(!mediaPlayer.isMetaDataAvailable())
    {
        QCoreApplication::processEvents();
    }
    //QStringList list = mediaPlayer.availableMetaData();

    //将解析到的歌曲信息存入类中
    if(mediaPlayer.isMetaDataAvailable())
    {
        //将StringList拼接成QString
        singer = mediaPlayer.metaData(QStringLiteral("Author")).toStringList().join(",");
        songTitle = mediaPlayer.metaData(QStringLiteral("Title")).toString();
        albumTitle = mediaPlayer.metaData(QStringLiteral("AlbumTitle")).toString();
        audioBitRate = mediaPlayer.metaData(QStringLiteral("AudioBitRate")).toInt();
//        while(mediaPlayer.duration() == 0)
//        {

//            QApplication::processEvents();

//        }


        duration = mediaPlayer.duration();
        //duration = mediaPlayer.duration();
        //qDebug() << "duration" << mediaPlayer.duration() << endl;
        //connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &Music::setDuration);
    }
}


void Music::setDuration()
{
    //duration = mediaPlayer.duration();
}


//获取歌词路径
QString Music::getLyricUrl() const
{
    QString musicLocalPath = url.toLocalFile();   //将url路径转换成本地文件路径
    //歌词文件与歌曲文件同名，将后缀替换即可
    musicLocalPath.replace(".mp3", ".lrc");         //TODO:此处可优化
    musicLocalPath.replace(".flac", ".lrc");
    musicLocalPath.replace(".mpga", "lrc");

    return musicLocalPath;
}


//返回歌曲的相关信息
QString Music::getMusicInfo() const
{
    return singer + "-" + songTitle + "[" + convertTimeFormat(duration) + "]";
}


//获取歌曲url
QUrl Music::getUrl() const
{
    return url;
}


//展示歌曲详细信息
void Music::showMusicDetailInfo()
{
    QString s("歌曲名：%1\n歌手：%2\n时长：%3\n唱片集：%4\n比特率：%5\n文件路径：%6");
    s = s.arg(songTitle, singer, convertTimeFormat(duration), albumTitle, QString::number(audioBitRate) + "bps", url.toString());
    QMessageBox::about(Q_NULLPTR, "歌曲信息", s);
}


//将歌曲信息插入数据库中
void Music::insertDatabase(const QString name)
{
    QSqlQuery mySqlQueue;
    //数据库查询命令,注意占位符？千万注意别少写了，不然无法插入数据
    QString queueCommand = "insert into MusicInfo values (?, ?, ?, ?, ?, ?, ?)";
    mySqlQueue.prepare(queueCommand);     //准备查询
    mySqlQueue.addBindValue(name);
    mySqlQueue.addBindValue(url.toString());
    mySqlQueue.addBindValue(singer);
    mySqlQueue.addBindValue(songTitle);
    mySqlQueue.addBindValue(duration);
    mySqlQueue.addBindValue(albumTitle);
    mySqlQueue.addBindValue(audioBitRate);

    mySqlQueue.exec();           //执行命令
//    {
//        qDebug() << "insert sql error!" << endl;

//    }
//    else
//    {
//        qDebug() << "insert sql successful!" << endl;
//        QString ss = "select * from MusicInfo";
//        mySqlQueue.exec();
//        while(mySqlQueue.next())
//        {
//            qDebug() << mySqlQueue.value(0).toString() << endl;
//        }
//    }
}


//设置歌曲排序方式
bool MusicSortMode::operator()(const Music &A, const Music &B)
{
    switch (key)
    {
        case Singer:
            return A.singer < B.singer;
        case SongTitle:
            return A.songTitle < B.songTitle;
        case Duration:
            return A.duration < B.duration;
        case Equality:
            return A.songTitle == B.songTitle && A.singer == B.singer;           //用来去重，歌名相同的歌曲被认为是同一首歌
        default:
            return A.getMusicInfo() < B.getMusicInfo();
    }
}













