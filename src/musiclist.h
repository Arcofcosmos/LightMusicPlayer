#ifndef MUSICLIST_H
#define MUSICLIST_H

#include <QVector>
#include <QList>
#include <QListWidget>
#include <QMediaPlaylist>

#include "music.h"


using namespace std;


class MusicListWidget;


//歌曲列表类
class MusicList
{

private:

    QString     musicListName;    //歌单名字
    QVector<Music>  musicList;    //该歌单存储的歌曲列表

    bool    sql_flag = true;         //表示是否与数据库交互

    friend class MusicListWidget;
    friend class MainWidget;

public:
    MusicList(){}
    MusicList(const QList<QUrl> &urlList, QString name = "");

    void    setName(QString name){musicListName = name;}          //设置歌单名
    QString getName(){return musicListName;}                      //获取歌单名
    void    setSqlFlag(bool flag){sql_flag = flag;}               //设置数据库标识
    void    addAllMusic(const QList<QUrl> urlList);               //添加全部歌曲到歌单
    void    addSingleMusic(const Music &music);                    //添加单首歌曲到歌单
    Music   getIndexMusic(int pos);                               //获取指定位置的歌曲
    //void    addToListWidget(QListWidget *listWidget);

    void    removeIndexMusic(int pos);                            //移除指定位置的歌曲
    void    openFolderOfIndexMusic(int pos);                      //打开指定位置歌曲所在的文件夹
    void    showMusicDetailInfo(int pos);                         //显示指定位置歌曲的详细信息
    void    removeSqlMusic();                                     //移除数据库中本歌单所有歌曲
    void    insertMusicToSql();                                   //将本歌单的歌曲放入数据库中
    void    readSqlMusic();                                       //从数据库中读取歌曲
    void    sortMusic(CompareMode key);                           //对歌单中歌曲进行排序
    void    clear();                                              //清空歌单
    void    neaten();                                             //去除歌单中重复歌曲并排序
    void    addToPlayList(QMediaPlaylist *playList);              //将歌单添加到播放列表中
    void    addToListWidget(MusicListWidget *listWidget);

};

#endif // MUSICLIST_H
















