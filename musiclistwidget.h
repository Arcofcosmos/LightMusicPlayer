#ifndef MUSICLISTWIDGET_H
#define MUSICLISTWIDGET_H

#include <QObject>
#include <QListWidget>
#include <QIcon>


#include "musiclist.h"


//该类需要歌单列表放入listWidget中管理，传送其组件给歌单
class MusicListWidget : public QListWidget
{
    Q_OBJECT

private:
    MusicList musicList;
    //当前列表中歌曲的图标
    QIcon icon = QIcon(":/image/image/image/music.png");

    QMediaPlayer *mediaPlayer;
public:
    MusicListWidget(QWidget *parent = Q_NULLPTR);

    void    refresh();              //当歌曲有变化时，刷新歌单

    void    setIcon(QIcon myIcon){icon = myIcon;}           //设置歌曲图标
    QIcon   getIcon(){return icon;}                         //获取歌曲图标

    void    removeMusic();                      //移除选中的歌曲
    void    showDetailInfo();                   //显示选中歌曲的详细信息
    void    openFolderOfMusic();                //打开选中歌曲的所在文件夹
    void    setMusicList(const MusicList myMusicList);                     //设置歌单列表
    void    setMusicList_Playing(const MusicList myMusciList);             //设置播放的歌曲列表

    friend class MainWidget;


signals:


public slots:

};


#endif // MUSICLISTWIDGET_H
