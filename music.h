#ifndef MUSIC_H
#define MUSIC_H


#include <QString>
#include <QUrl>
#include <QMediaPlayer>
#include <QObject>
#include <QWidget>



class Music;

//歌曲排序模式
enum CompareMode{Default, Singer, SongTitle, Duration, Equality};

//按照不同key值对歌曲进行排序
class MusicSortMode
{
    CompareMode    key;        //歌曲排序模式

public:
    MusicSortMode(){key = Default;}
    MusicSortMode(CompareMode myKey){key = myKey;}
    bool operator()(const Music &A, const Music &B);           //重载"()"按照不同模式排序歌曲
};


//单首歌曲类
class Music
{

private:

    /*歌曲信息*/
    QUrl        url;            //歌曲存储路径
    QString     singer;         //歌手
    QString     songTitle;      //歌名
    qint64      duration;       //歌曲时长
    QString     albumTitle;     //专辑名
    int         audioBitRate;   //比特率

    //QMediaPlayer mediaPlayer;

    void    analysisMusic();    //根据歌曲路径获取歌曲并解析歌曲信息

    friend class MusicSortMode;
    friend class MusicList;
    friend class MainWidget;
    friend class MusicListWidget;

public:
    Music(){}
    Music(QUrl myUrl);



    QUrl    getUrl() const;
    QString    getLyricUrl() const;      //根据歌曲路径获取同名的歌词路径
    QString    getMusicInfo() const;      //返回歌曲的相关信息
    void    insertDatabase(const QString name);       //将歌曲信息插入数据库中
    void    showMusicDetailInfo();                    //展示歌曲详细信息

public slots:
    void setDuration();                       //设置歌曲时长

};


#endif // MUSIC_H
